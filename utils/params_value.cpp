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

#include "params_value.h"

#include "utils/io/stream.h"

namespace Imagine
{

void ParamsValue::load(Stream* stream)
{
	m_type = (Type)stream->loadEnum();

	switch (m_type)
	{
		case eBool:
			stream->loadBool(m_bool);
			break;
		case eUInt:
			stream->loadUInt(m_unsignedInt);
			break;
		case eFloat:
			stream->loadFloat(m_float);
			break;
		default:
			break;
	}
}

void ParamsValue::store(Stream* stream) const
{
	stream->storeEnum((unsigned int)m_type);

	switch (m_type)
	{
		case eBool:
			stream->storeBool(m_bool);
			break;
		case eUInt:
			stream->storeUInt(m_unsignedInt);
			break;
		case eFloat:
			stream->storeFloat(m_float);
			break;
		default:
			break;
	}
}

} // namespace Imagine
