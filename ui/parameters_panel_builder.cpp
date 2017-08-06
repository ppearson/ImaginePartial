/*
 Imagine
 Copyright 2012-2016 Peter Pearson.

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

#include "parameters_panel_builder.h"

#include "parameter.h"
#include "parameters_panel.h"

#include "controls/string_control.h"
#include "controls/float_control.h"
#include "controls/uint_control.h"
#include "controls/int_control.h"
#include "controls/float_slider_control.h"
#include "controls/enum_control.h"
#include "controls/colour_control.h"
#include "controls/bool_control.h"
#include "controls/vector_control.h"
#include "controls/animated_vector_control.h"
#include "controls/material_control.h"
#include "controls/file_control.h"
#include "controls/float_pair_control.h"
#include "controls/animated_float_control.h"
#include "controls/texture_control.h"
#include "controls/ray_visibility_control.h"
#include "controls/material_edit_control.h"
#include "controls/transform_control.h"

namespace Imagine
{

ParametersPanelBuilder::ParametersPanelBuilder()
{
}

ParametersPanel* ParametersPanelBuilder::buildParametersPanel(Parameters& parameters, ParametersInterface* pParent, ParameterPanelType panelType)
{
	std::vector<Parameter*>& params = parameters.getParameters();
	if (params.empty())
		return NULL;

	ParametersPanel* pPP = new ParametersPanel(pParent, panelType);

	pPP->addTab(panelType == eObjectParameter ? "Object" : "Basic");

	std::vector<Parameter*>::iterator it = params.begin();
	for (; it != params.end(); ++it)
	{
		Parameter* pParam = *it;

		ParameterType parameterType = pParam->getType();
		std::string name = pParam->getName();
		std::string label = pParam->getLabel();
		unsigned int flags = pParam->getFlags();

		if (panelType == eMaterialParameter)
			flags |= eParameterFloatSliderEdit;

		// this isn't great, but due to wanting a completely clean separation between ParameterPanels/Controls and Parameters, the easiest
		// way to do things...

		Control* pControl = NULL;

		bool fullControl = true;
		bool addLabel = true;

		switch (parameterType)
		{
			case eParameterBool:
			{
				BasicParameter<bool>* pTypedParam = static_cast<BasicParameter<bool>*>(pParam);
				pControl = new BoolControl(name, pTypedParam->getPairedValue(), label);
				break;
			}
			case eParameterUInt:
			{
				RangeParameter<unsigned int, unsigned int>* pTypedParam = static_cast<RangeParameter<unsigned int, unsigned int>*>(pParam);
				bool scrub = pTypedParam->getFlags() & eParameterScrubButton;
				pControl = new UIntControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label, scrub);
				break;
			}
			case eParameterInt:
			{
				RangeParameter<int, int>* pTypedParam = static_cast<RangeParameter<int, int>*>(pParam);
				bool scrub = pTypedParam->getFlags() & eParameterScrubButton;
				pControl = new IntControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label, scrub);
				break;
			}
			case eParameterFloat:
			{
				RangeParameter<float, float>* pTypedParam = static_cast<RangeParameter<float, float>*>(pParam);
				pControl = new FloatControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label,
											pTypedParam->getFlags());
				break;
			}
			case eParameterFloatSlider:
			{
				RangeParameter<float, float>* pTypedParam = static_cast<RangeParameter<float, float>*>(pParam);

				bool editControl = (flags & eParameterFloatSliderEdit);
				bool logScale = (flags & eParameterFloatSliderLogScale);
				bool highPrecision = (flags & eParameterFloatSliderHighPrecision);

				pControl = new FloatSliderControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(),
												  label, editControl, logScale, highPrecision);
				break;
			}
			case eParameterFloatPair:
			{
				RangeParameterPair<float, float>* pTypedParam = static_cast<RangeParameterPair<float, float>*>(pParam);
				FloatPairControl* pActualControl = new FloatPairControl(name, pTypedParam->getPairedValue1(), pTypedParam->getPairedValue2(),
												 pTypedParam->getMin(), pTypedParam->getMax(), label, pTypedParam->getFlags());

				pActualControl->setTooltips(pTypedParam->getIndividualLabel1(), pTypedParam->getIndividualLabel2());

				pControl = pActualControl;

				break;
			}
			case eParameterFloatSplitPair:
			{
				RangeParameterSplitPair<float, float>* pTypedParam = static_cast<RangeParameterSplitPair<float, float>*>(pParam);
				pControl = new FloatPairControl(name, pTypedParam->getPairedValue1(), pTypedParam->getPairedValue2(), pTypedParam->getMin1(),
												 pTypedParam->getMax1(), pTypedParam->getMin2(), pTypedParam->getMax2(), label, pTypedParam->getFlags());
				break;
			}
			case eParameterString:
			{
				BasicParameter<std::string>* pTypedParam = static_cast<BasicParameter<std::string>*>(pParam);
				pControl = new StringControl(name, pTypedParam->getPairedValue(), label);
				break;
			}
			case eParameterEnum:
			{
				EnumParameter* pTypedParam = static_cast<EnumParameter*>(pParam);
				pControl = new EnumControl(name, pTypedParam->getPairedValue(), pTypedParam->getOptions(), label);
				break;
			}
			case eParameterColour:
			{
				BasicParameter<Colour3f>* pTypedParam = static_cast<BasicParameter<Colour3f>*>(pParam);
				pControl = new ColourControl(name, pTypedParam->getPairedValue(), label);
				break;
			}
			case eParameterVector:
			{
				RangeParameter<Vector, float>* pTypedParam = static_cast<RangeParameter<Vector, float>*>(pParam);
				pControl = new VectorControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label);
				break;
			}
			case eParameterAnimatedVector:
			{
				RangeParameter<AnimatedVector, float>* pTypedParam = static_cast<RangeParameter<AnimatedVector, float>*>(pParam);
				pControl = new AnimatedVectorControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label);
				break;
			}
			case eParameterAnimatedFloat:
			{
				RangeParameter<AnimationCurve, float>* pTypedParam = static_cast<RangeParameter<AnimationCurve, float>*>(pParam);
				bool scrub = pTypedParam->getFlags() & eParameterScrubButton;
				pControl = new AnimatedFloatControl(name, pTypedParam->getPairedValue(), pTypedParam->getMin(), pTypedParam->getMax(), label, scrub);
				break;
			}
			case eParameterMaterial:
			{
				BasicParameter<Object>* pTypedParam = static_cast<BasicParameter<Object>*>(pParam);

				// TODO: this is a quick hack...
				if (flags & eParameterMaterialSelectionManager)
				{
					pControl = new MaterialControl(name, NULL, label);
					MaterialControl* pMC = static_cast<MaterialControl*>(pControl);
					pMC->setSelectionManager();
				}
				else
				{
					// single object
					pControl = new MaterialControl(name, pTypedParam->getPairedValue(), label);
				}
				break;
			}
			case eParameterFile:
			{
				BasicParameter<std::string>* pTypedParam = static_cast<BasicParameter<std::string>*>(pParam);
				// flags for type of category
				FileControl::FileCategory category = FileControl::eNormal;
				if (flags & eParameterFileParamTexture)
					category = FileControl::eTexture;
				else if (flags & eParameterFileParamEnvMap)
					category = FileControl::eEnvironmentMap;
				else if (flags & eParameterFileParamVolumeBuffer)
					category = FileControl::eVolumeBuffer;

				pControl = new FileControl(name, pTypedParam->getPairedValue(), label, category);
				break;
			}
			case eParameterTexture1f:
			case eParameterTexture3f:
			{
				BasicParameter<TextureParameters>* pTypedParam = static_cast<BasicParameter<TextureParameters>*>(pParam);

				pControl = new TextureControl(name, pTypedParam->getPairedValue(), label, flags);
				break;
			}
			case eParameterBitmask:
			{
				// this is a bit naughty, but for the moment easier...
				BitmaskParameter* pTypedParam = static_cast<BitmaskParameter*>(pParam);
				pControl = new RayVisibilityControl(name, pTypedParam->getPairedValue(), label);

				break;
			}
			case eParameterMaterialEdit:
			{
				BasicParameter<Material*>* pTypedParam = static_cast<BasicParameter<Material*>*>(pParam);

				pControl = new MaterialEditControl(name, pTypedParam->getPairedValue(), label);

				break;
			}
			case eParameterGroup:
			{
				GroupParameter* pTypedParam = static_cast<GroupParameter*>(pParam);
				pPP->addTab(pTypedParam->getLabel());
				delete pParam;
				continue;
			}
			case eParameterSeparator:
			{
				fullControl = false;
				break;
			}
			case eParameterTransform:
			{
				addLabel = false;

				BasicParameter<Transform>* pTypedParam = static_cast<BasicParameter<Transform>*>(pParam);

				pControl = new TransformControl(name, pTypedParam->getPairedValue());

				break;
			}
		}

		if (fullControl)
		{
			if (addLabel)
			{
				pPP->addControl(pControl, true, pParam->getGroup());
			}
			else
			{
				pPP->addControl(pControl, false, pParam->getGroup());
			}
		}
		else
		{
			pPP->addDescriptorLine(label, pParam->getGroup());
		}

		delete pParam;
	}

	// now hide any initially hidden parameters
	const std::set<std::string>& hiddenParams = parameters.getInitiallyHiddenParameters();
	std::set<std::string>::const_iterator itHiddenParam = hiddenParams.begin();
	for (; itHiddenParam != hiddenParams.end(); ++itHiddenParam)
	{
		const std::string& name = *itHiddenParam;

		pPP->hideControl(name);
	}

	return pPP;
}

} // namespace Imagine
