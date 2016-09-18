/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

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

#include "control_connection_proxy.h"

#include "controls/control.h"

namespace Imagine
{

ControlConnectionProxy::ControlConnectionProxy(Control* pOwner) :
	QObject(), m_pOwner(pOwner)
{
}

void ControlConnectionProxy::registerValueChangedDouble(QObject* sender, int index)
{
	QObject::connect(sender, SIGNAL(editingFinished()), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerSliderMovedInt(QObject* sender, int index)
{
	QObject::connect(sender, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
}

void ControlConnectionProxy::registerComboIndexChangedInt(QObject *sender)
{
	QObject::connect(sender, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerButtonClicked(QObject* sender)
{
	QObject::connect(sender, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

void ControlConnectionProxy::registerButtonClicked(QObject* sender, unsigned int index)
{
	m_aButtonIndexes[sender] = index;
	QObject::connect(sender, SIGNAL(clicked()), this, SLOT(buttonClickedIndex()));
}

void ControlConnectionProxy::registerItemChanged(QObject* sender)
{
	QObject::connect(sender, SIGNAL(changed()), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerEditingFinished(QObject* sender)
{
	QObject::connect(sender, SIGNAL(editingFinished()), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerCheckboxToggled(QObject* sender)
{
	QObject::connect(sender, SIGNAL(stateChanged(int)), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerMenuSelected(QObject* sender)
{
	QObject::connect(sender, SIGNAL(mapped(int)), this, SLOT(menuSelected(int)));
}

void ControlConnectionProxy::registerDeltaChange(QObject* sender, unsigned int index)
{
	m_aDeltaChangeIndexes[sender] = index;
	QObject::connect(sender, SIGNAL(deltaMove(float)), this, SLOT(deltaChange(float)));
}

void ControlConnectionProxy::registerTextureChanged(QObject* sender)
{
	QObject::connect(sender, SIGNAL(textureChanged()), this, SLOT(valueChanged()));
}

void ControlConnectionProxy::registerRayVisButtonSelChanged(QObject* sender)
{
	QObject::connect(sender, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

void ControlConnectionProxy::valueChanged()
{
	if (m_pOwner)
	{
		// call Control-specific version to update paired value
		if (m_pOwner->valueChanged())
		{
			// if the value actually did change (up to each control to check this or not)

			// notify Parameter Panel that control was changed
			m_pOwner->sendValueChanged();
		}
	}
}

void ControlConnectionProxy::sliderChanged(int value)
{
	if (m_pOwner)
	{
		// call Control-specific version to update paired value
		if (m_pOwner->sliderChanged(value))
		{
			// if the value actually did change (up to each control to check this or not)

			// notify Parameter Panel that control was changed
			m_pOwner->sendValueChanged();
		}
	}
}

void ControlConnectionProxy::buttonClicked()
{
	if (m_pOwner)
	{
		if (m_pOwner->buttonClicked(0))
		{
			m_pOwner->sendValueChanged();
		}
	}
}

void ControlConnectionProxy::buttonClickedIndex()
{
	if (m_pOwner)
	{
		QObject* pSender = sender();
		unsigned int index = m_aButtonIndexes[pSender];
		if (m_pOwner->buttonClicked(index))
		{
			m_pOwner->sendValueChanged();
		}
	}
}

void ControlConnectionProxy::menuSelected(int index)
{
	if (m_pOwner)
	{
		m_pOwner->menuSelected(index);

		m_pOwner->sendValueChanged();
	}
}

void ControlConnectionProxy::deltaChange(float delta)
{
	if (m_pOwner)
	{
		QObject* pSender = sender();
		unsigned int index = m_aDeltaChangeIndexes[pSender];
		if (m_pOwner->deltaChange(delta, index))
		{
			// if the value actually did change (up to each control to check this or not)
			m_pOwner->sendValueChanged();
		}
	}
}

void ControlConnectionProxy::selectionChanged()
{
	if (m_pOwner)
	{
		if (m_pOwner->valueChanged())
		{
			m_pOwner->sendValueChanged();
		}
	}
}

} // namespace Imagine
