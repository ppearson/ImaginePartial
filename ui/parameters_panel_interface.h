/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#ifndef PARAMETERS_PANEL_INTERFACE_H
#define PARAMETERS_PANEL_INTERFACE_H

#include <string>

namespace Imagine
{

class ParametersPanelInterface
{
public:

	virtual void controlChanged(const std::string& name) = 0;
};

} // namespace Imagine

#endif // PARAMETERS_PANEL_INTERFACE_H
