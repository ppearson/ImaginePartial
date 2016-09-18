/*
 Imagine
 Copyright 2015 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#ifndef SHARDED_MAP_H
#define SHARDED_MAP_H

#include <vector>
#include <map>

// clang on OS X can use std::unordered_map without -std=c++11, so use this corner case for the moment
// so we can build on both Linux and OS X, and still use GCC 4.4 on Linux without C++11 support
#if ENABLE_UNORDERED_MAP
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__) || defined(__clang__)
#include <unordered_map>
using std::unordered_map;
#elif defined(__linux__)
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif
#endif

#include "core/hash.h"

#include "utils/threads/mutex.h"

namespace Imagine
{
//#include "utils/spin_lock.h" // is actually slower than just a standard mutex on Linux

// A Sharded Map class, which splits the map into "shards" (I've used the DB name for
// splitting/distributing, as opposed to 'bins' which Java's ConcurrentHashMap uses)
// or horizontal splits, each of which contains separate maps and locks which can be
// accessed in a distributed fashion based off the modulus of the hash value.
// This allows orders of magnitude faster concurrent access in a multi-threaded
// environment, as there's no single lock contention to bottleneck access.

// Currently, this is not a full implementation of a map (doesn't support all uses cases of a map)
// and isn't a drop-in replacement, as the iterators contain locking semantics which make certain things
// ([] operators) very tricky to do in the same way - i.e. provide direct access to the .second items.

// It's also only currently templated on the store value type, not the key

// store adapters, containing the actual associative containers themselves
template <class Value>
class SARBMap
{
public:
	// Standard Map (RB Tree) implementation
	SARBMap()
	{
	}

	typedef std::map<HashValue, Value> StoreType_t;
	typedef typename StoreType_t::iterator	iterator;

	void init()
	{
	}

	iterator itBegin()
	{
		return m_store.begin();
	}

	iterator itEnd()
	{
		return m_store.end();
	}

	bool isEmpty()
	{
		return m_store.empty();
	}

	void clear()
	{
		m_store.clear();
	}

	iterator find(HashValue value)
	{
		return m_store.find(value);
	}

	iterator insertScoped(const std::pair<HashValue, Value>& values)
	{
		return m_store.insert(values).first;
	}

	iterator erase(const iterator itErase)
	{
		return m_store.erase(itErase);
	}

protected:
	std::map<HashValue, Value>	m_store;
};

#if ENABLE_UNORDERED_MAP
template <class Value>
class SAUnorderedMap
{
public:
	// Unordered map implementation
	SAUnorderedMap()
	{
	}

	typedef unordered_map<HashValue, Value> StoreType_t;
	typedef typename StoreType_t::iterator	iterator;

	void init()
	{
	}

	iterator itBegin()
	{
		return m_store.begin();
	}

	iterator itEnd()
	{
		return m_store.end();
	}

	bool isEmpty()
	{
		return m_store.empty();
	}

	void clear()
	{
		m_store.clear();
	}

	iterator find(HashValue value)
	{
		return m_store.find(value);
	}

	iterator insertScoped(const std::pair<HashValue, Value>& values)
	{
		return m_store.insert(values).first;
	}

	iterator erase(const iterator itErase)
	{
		return m_store.erase(itErase);
	}

protected:
	StoreType_t	m_store;
};
#endif

template <class Value, class StoreAdapter>
class ShardedMap
{
public:
	ShardedMap() : m_shardCount(1)
	{
		m_aShards.resize(1);
	}

	~ShardedMap()
	{
	}

	// this should only really be called once up-front...
	void init(unsigned int numShards)
	{
		m_shardCount = numShards;

		m_aShards.resize(numShards);
	}

public:
	class iterator
	{
	public:
		iterator() : m_pShardMap(NULL), m_shardIndex(-1), m_holdLock(false)
		{
		}

		iterator(ShardedMap* pOwner, unsigned int shardIndex) : m_pShardMap(pOwner), m_shardIndex(shardIndex), m_holdLock(false)
		{
		}

		iterator(const iterator& rhs)
		{
			m_pShardMap = rhs.m_pShardMap;
			m_storeIt = rhs.m_storeIt;
			m_shardIndex = rhs.m_shardIndex;
			m_holdLock = rhs.m_holdLock;

			rhs.m_holdLock = false;
		}

		~iterator()
		{
			releaseShard();
			m_pShardMap = NULL;
		}

		iterator& operator=(const iterator& rhs)
		{
			// we intentionally transfer lock ownership here
			m_pShardMap = rhs.m_pShardMap;
			m_storeIt = rhs.m_storeIt;
			m_shardIndex = rhs.m_shardIndex;
			m_holdLock = rhs.m_holdLock;

			rhs.m_holdLock = false;

			return *this;
		}

		typename StoreAdapter::StoreType_t::value_type& operator*()
		{
			return *m_storeIt;
		}

		bool operator==(const iterator& rhs)
		{
			// for end() comparisons
			if (m_shardIndex == -1u && rhs.m_shardIndex == -1u)
				return true;

			if (m_pShardMap != rhs.m_pShardMap)
				return false;

			if (m_shardIndex != rhs.m_shardIndex)
				return false;

			if (m_storeIt != rhs.m_storeIt)
				return false;

			return true;
		}

		bool operator!=(const iterator& rhs)
		{
			return !(*this == rhs);
		}

		void operator++()
		{
			++m_storeIt;

			// continue through shards, skipping any empty ones...
			while (m_storeIt == m_pShardMap->m_aShards[m_shardIndex].m_store.itEnd())
			{
				if (m_shardIndex == (m_pShardMap->getShardCount() - 1))
				{
					// we've reached the end, so bail out
					releaseShard();
					return;
				}

				aquireShard(m_shardIndex + 1);
			}
		}

		friend class ShardedMap<Value, StoreAdapter>;

		void lock()
		{
			if (m_holdLock)
				return;

			if (m_shardIndex == -1u)
				return;

			Shard& shardItem = m_pShardMap->m_aShards[m_shardIndex];
			shardItem.m_lock.lock();

			m_holdLock = true;
		}

		void unlock()
		{
			if (!m_holdLock)
				return;

			if (m_shardIndex == -1u)
				return;

			Shard& shardItem = m_pShardMap->m_aShards[m_shardIndex];
			shardItem.m_lock.unlock();

			m_holdLock = false;
		}

		unsigned int getShardIndex() const
		{
			return m_shardIndex;
		}

	protected:
		void aquireShard(unsigned int shardIndex)
		{
			releaseShard();

			m_shardIndex = shardIndex;

			lock();

			Shard& shardItem = m_pShardMap->m_aShards[m_shardIndex];
			m_storeIt = shardItem.m_store.itBegin();
		}

		void releaseShard()
		{
			if (m_shardIndex == -1u)
			{
				return;
			}

			if (m_holdLock)
			{
				unlock();
			}

			m_shardIndex = -1u;
		}

		// checks whether the internal iterator to an item in a shard's map is valid
		// assumes lock for shard is held
		bool isValid() const
		{
			Shard& shardItem = m_pShardMap->m_aShards[m_shardIndex];
			// TODO: this weirdly doesn't seem to work in all cases - end() seems to be different between std::map
			//       and std::unordered_map after incrementing an end() iterator...
			return (m_storeIt != shardItem.m_store.itEnd());
		}

		// assumes lock for shard is held
		bool containsItems() const
		{
			Shard& shardItem = m_pShardMap->m_aShards[m_shardIndex];
			return !shardItem.m_store.isEmpty();
		}

	protected:
		ShardedMap*			m_pShardMap;
		typename StoreAdapter::iterator	m_storeIt;
		unsigned int		m_shardIndex;
		mutable bool		m_holdLock;
	};

	// return an iterator pointing to the first item in a shard.
	// Note: this may not point to the first shard, and if the entire
	//       map is empty, it will return the same as end()
	iterator begin()
	{
		iterator it(this, -1u);
		it.aquireShard(0);

		// find the first shard with items in it
		while (!it.containsItems())
		{
			if (it.m_shardIndex == (m_shardCount - 1))
			{
				// we've reached the end, so bail out
				it.releaseShard();
				return end();
			}

			// otherwise, switch to the next shard
			it.aquireShard(it.m_shardIndex + 1);
		}

		if (!it.containsItems())
			return end();

		return it;
	}

	iterator end()
	{
		// don't care about shard index here, as it's an invalid iterator anyway...
		return iterator();
	}

	// ideally, this would be const, but...
	iterator find(HashValue value)
	{
		// mix the hash value, and use the mixed result to work out the shard
		// this is important so that we get good distribution of keys for both the shards
		// and the storage maps, and helps prevent clustering
		HashValue shardHash = mixHash(value);
		unsigned int shardIndex = getShardIndex(shardHash);

		Shard& shard = m_aShards[shardIndex];
		shard.m_lock.lock();

		// now try and find item in inner map
		typename StoreAdapter::iterator itFind = shard.m_store.find(value);
		if (itFind == shard.m_store.itEnd())
		{
			// we don't have it
			shard.m_lock.unlock();
			return end();
		}

		// we have it, so create an iterator
		iterator itResult(this, shardIndex);
		itResult.m_storeIt = itFind;
		itResult.m_holdLock = true;

		return itResult;
	}

	// return an iterator pointing to the start of the shard that the hashvalue
	// should belong in, or the next non-empty shard if the target one is empty
	iterator itShardForHash(HashValue value)
	{
		// mix the hash value, and use the mixed result to work out the shard
		// this is important so that we get good distribution of keys for both the shards
		// and the storage maps, and helps prevent clustering
		HashValue shardHash = mixHash(value);
		unsigned int shardIndex = getShardIndex(shardHash);

		iterator it(this, -1u);
		it.aquireShard(shardIndex);

		// find the first shard with items in it
		while (!it.containsItems())
		{
			if (it.m_shardIndex == (m_shardCount - 1))
			{
				// we've reached the end, so bail out
				it.releaseShard();
				return end();
			}

			// otherwise, switch to the next shard
			it.aquireShard(it.m_shardIndex + 1);
		}

		if (!it.containsItems())
			return end();

		return it;
	}

	// return iterator pointing to the beginning of the specified shard, or the next non-empty shard
	// if the specified shard was empty
	iterator itForShardIndex(unsigned int shardIndex)
	{
		iterator it(this, -1u);
		it.aquireShard(shardIndex % m_shardCount);

		// find the first shard with items in it
		while (!it.containsItems())
		{
			if (it.m_shardIndex == (m_shardCount - 1))
			{
				// we've reached the end, so bail out
				it.releaseShard();
				return end();
			}

			// otherwise, switch to the next shard
			it.aquireShard(it.m_shardIndex + 1);
		}

		if (!it.containsItems())
			return end();

		return it;
	}

	// currently, the usage of this is designed that there be no existing
	// key in the store already - i.e. find() has been called previously, and insert()
	// is called based on find() returning an invalid iterator...

	// returns an iterator which holds the lock on the shard
	// until the iterator goes out of scope
	iterator insert(std::pair<HashValue, Value> values)
	{
		// mix the hash value, and use the mixed result to work out the shard
		// this is important so that we get good distribution of keys for both the shards
		// and the storage maps, and helps prevent clustering
		HashValue shardHash = mixHash(values.first);

		unsigned int shardIndex = getShardIndex(shardHash);

		Shard& shard = m_aShards[shardIndex];
		shard.m_lock.lock();

		typename StoreAdapter::StoreType_t::iterator itNewItem = shard.m_store.insertScoped(values);

		// we have it, so create an iterator
		iterator itResult(this, shardIndex);
		itResult.m_storeIt = itNewItem;
		itResult.m_holdLock = true;

		return itResult;
	}

	// erases the item the iterator points to, and returns an iterator to the next item in the shard's
	// map, or the first item in the next shard if the deleted item was the last item in the shard...
	iterator erase(iterator& itErase)
	{
		// assume here that itErase holds the lock

		Shard& shard = m_aShards[itErase.getShardIndex()];

		typename StoreAdapter::StoreType_t::iterator itEraseMapResult = shard.m_store.erase(itErase.m_storeIt);

		iterator itResult;

		// if we've now got a resulting valid iterator from the erase operation, we should have the next item
		// after the one we've just deleted.
		if (itEraseMapResult != shard.m_store.itEnd())
		{
			// get iterator within the shard's map, and return that
			itResult = itErase;
			itResult.m_storeIt = itEraseMapResult;
			return itResult;
		}

		// otherwise, we've run off the end of the shard, so try and aquire next shard

		if (itErase.m_shardIndex == (m_shardCount - 1))
		{
			itErase.releaseShard();
			// we're the last shard, so bail out returning an invalid iterator as there's no more shards or items
			return end();
		}

		// try the next shard - this will either be in the next item in any next shard, or the end() iterator

		itErase.releaseShard();
		itResult = itForShardIndex(itErase.m_shardIndex + 1);

		return itResult;
	}

	// this functions in the same way as clear() on std::map
	void clear()
	{
		for (unsigned int i = 0; i < m_shardCount; i++)
		{
			Shard& shard = m_aShards[i];

			shard.m_lock.lock();
			shard.m_store.clear();
			shard.m_lock.unlock();
		}
	}

protected:
	unsigned int getShardIndex(HashValue value) const
	{
		return (unsigned int)(value % m_shardCount);
	}

	unsigned int getShardCount() const
	{
		return m_shardCount;
	}

	// taken from MurmurHash3
	static HashValue mixHash(HashValue val)
	{
		val ^= val >> 33;
		val *= uint64_t(0xff51afd7ed558ccd);
		val ^= val >> 33;
		val *= uint64_t(0xc4ceb9fe1a85ec53);
		val ^= val >> 33;

		return val;
	}

	// it's important that these are in a struct, so the items for each shard are not interleaved which causes false-sharing
	class Shard
	{
	public:
		StoreAdapter	m_store;
		// this is quite large on linux, but using a spin lock doesn't seem to be any faster, so I don't think it matters. On other
		// platforms where Futexes don't exist, this might be a different story...
		Mutex			m_lock;
	};

	std::vector<Shard>	m_aShards;

	unsigned int		m_shardCount;
};

} // namespace Imagine

#endif // SHARDED_MAP_H

