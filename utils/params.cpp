/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#include "params.h"

#include "utils/io/stream.h"

namespace Imagine
{

void Params::load(Stream* stream)
{
	m_aParams.clear();

	unsigned int numItems = 0;
	stream->loadUIntFromUChar(numItems);

	for (unsigned int i = 0; i < numItems; i++)
	{
		std::string name;
		stream->loadString(name);

		ParamsValue newValue;
		newValue.load(stream);

		m_aParams[name] = newValue;
	}
}

void Params::store(Stream* stream) const
{
	unsigned int numItems = m_aParams.size();

	stream->storeUIntAsUChar(numItems);

	std::map<std::string, ParamsValue>::const_iterator it = m_aParams.begin();
	for (; it != m_aParams.end(); ++it)
	{
		const std::string& name = (*it).first;

		stream->storeString(name);

		const ParamsValue& value = (*it).second;

		value.store(stream);
	}
}

void Params::mergeOverwrite(const Params& params)
{
	std::map<std::string, ParamsValue>::const_iterator it = params.m_aParams.begin();
	for (; it != params.m_aParams.end(); ++it)
	{
		const std::string& name = (*it).first;
		const ParamsValue& value = (*it).second;

		m_aParams[name] = value;
	}
}

bool Params::hasKey(const std::string& key) const
{
	std::map<std::string, ParamsValue>::const_iterator itFind = m_aParams.find(key);
	return itFind != m_aParams.end();
}

} // namespace Imagine
