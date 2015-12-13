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

#ifndef MANIPULATOR_HANDLE_H
#define MANIPULATOR_HANDLE_H

#include <string>

#include "core/point.h"

class Object;
class Vector;

class ManipulatorHandle
{
public:
	ManipulatorHandle(Object* pObject, const std::string& name) : m_pObject(pObject), m_name(name)
	{
	}
    
    virtual ~ManipulatorHandle() { }

	virtual void draw() = 0;

	virtual void applyDelta(Vector& delta) = 0;
	virtual Point getCentreOfPosition() = 0;

protected:
	Object*			m_pObject;
	std::string		m_name;
};


class Position3DManipulatorHandle : public ManipulatorHandle
{
public:
	Position3DManipulatorHandle(Object* pObject, const std::string& name, Vector& pairedValue) : ManipulatorHandle(pObject, name),
		m_pairedValue(pairedValue)
	{

	}

	virtual void draw();

	virtual void applyDelta(Vector& delta);
	virtual Point getCentreOfPosition();

protected:
	Vector&		m_pairedValue;
};

#endif // MANIPULATOR_HANDLE_H
