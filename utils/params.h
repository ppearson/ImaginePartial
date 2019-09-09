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

#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <map>

#include "params_value.h"

// Simple (and by-design limited) class to provide collections of fixed values that can be looked up
// by name

namespace Imagine
{

class Stream;

class Params
{
public:
	Params()
	{
	}

	void add(const std::string& name, bool value)
	{
		m_aParams[name] = ParamsValue(value);
	}

	void add(const std::string& name, int value)
	{
		m_aParams[name] = ParamsValue((unsigned int)value);
	}

	void add(const std::string& name, unsigned int value)
	{
		m_aParams[name] = ParamsValue(value);
	}

	void add(const std::string& name, float value)
	{
		m_aParams[name] = ParamsValue(value);
	}
	
	void addIfNotSet(const std::string& name, bool value)
	{
		if (m_aParams.find(name) == m_aParams.end())
			m_aParams[name] = ParamsValue(value);
	}

	void addIfNotSet(const std::string& name, int value)
	{
		if (m_aParams.find(name) == m_aParams.end())
			m_aParams[name] = ParamsValue((unsigned int)value);
	}

	void addIfNotSet(const std::string& name, unsigned int value)
	{
		if (m_aParams.find(name) == m_aParams.end())
			m_aParams[name] = ParamsValue(value);
	}

	void addIfNotSet(const std::string& name, float value)
	{
		if (m_aParams.find(name) == m_aParams.end())
			m_aParams[name] = ParamsValue(value);
	}

	bool getBool(const std::string& name, bool defaultValue = false) const
	{
		std::map<std::string, ParamsValue>::const_iterator it = m_aParams.find(name);
		if (it == m_aParams.end() || it->second.getType() != ParamsValue::eBool)
			return defaultValue;

		return it->second.getBool();
	}

	unsigned int getUInt(const std::string& name, unsigned int defaultValue = 0) const
	{
		std::map<std::string, ParamsValue>::const_iterator it = m_aParams.find(name);
		if (it == m_aParams.end() || it->second.getType() != ParamsValue::eUInt)
			return defaultValue;

		return it->second.getUInt();
	}

	float getFloat(const std::string& name, float defaultValue = 0.0f) const
	{
		std::map<std::string, ParamsValue>::const_iterator it = m_aParams.find(name);
		if (it == m_aParams.end() || it->second.getType() != ParamsValue::eFloat)
			return defaultValue;

		return it->second.getFloat();
	}
	
	bool isEmpty() const
	{
		return m_aParams.empty();
	}

	void load(Stream* stream);
	void store(Stream* stream) const;

	void mergeOverwrite(const Params& params);

	bool hasKey(const std::string& key) const;

protected:
	std::map<std::string, ParamsValue>	m_aParams;
};

} // namespace Imagine

#endif // PARAMS_H
