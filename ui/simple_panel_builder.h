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

#ifndef SIMPLE_PANEL_BUILDER_H
#define SIMPLE_PANEL_BUILDER_H

#include "parameter.h"

class Parameters;
class SimpleParametersPanel;
class QWidget;

//! Similar to ParametersPanelBuilder, but doesn't add to tabs...

class SimplePanelBuilder
{
public:
	SimplePanelBuilder();

	static SimpleParametersPanel* buildParametersPanel(Parameters& parameters, ParametersInterface* pParent, ParameterPanelType panelType);
};

#endif // SIMPLE_PANEL_BUILDER_H
