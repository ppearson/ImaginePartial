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

#ifndef PARAMS_VALUE_H
#define PARAMS_VALUE_H

// extremely simple variant value type for params with Params class - intended for internal usage of single params.
// for more flexible version, use ArgValue / Args classes instead

namespace Imagine
{

class Stream;

class ParamsValue
{
public:
	enum Type
	{
		eNone,
		eBool,
		eUInt,
		eFloat
	};

	ParamsValue() : m_type(eNone)
	{
	}

	explicit ParamsValue(bool value) : m_type(eBool), m_bool(value)
	{
	}

	explicit ParamsValue(unsigned int value) : m_type(eUInt), m_unsignedInt(value)
	{
	}

	explicit ParamsValue(int value) : m_type(eUInt), m_unsignedInt(value)
	{
	}

	explicit ParamsValue(float value) : m_type(eFloat), m_float(value)
	{
	}

	Type getType() const
	{
		return m_type;
	}

	bool getBool() const
	{
		return m_bool;
	}

	unsigned int getUInt() const
	{
		return m_unsignedInt;
	}

	float getFloat() const
	{
		return m_float;
	}

	void load(Stream* stream);
	void store(Stream* stream) const;

protected:
	Type				m_type;
	union
	{
		bool			m_bool;
		unsigned int	m_unsignedInt;
		float			m_float;
	};
};

} // namespace Imagine

#endif // PARAMS_VALUE_H
