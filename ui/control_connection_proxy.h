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

#ifndef CONTROL_CONNECTION_PROXY_H
#define CONTROL_CONNECTION_PROXY_H

#include <QObject>

#include <map>

class Control;

class ControlConnectionProxy : public QObject
{
	Q_OBJECT
public:
	ControlConnectionProxy(Control* pOwner);

	void registerValueChangedDouble(QObject* sender, int index = -1);
	void registerSliderMovedInt(QObject* sender, int index = -1);
	void registerComboIndexChangedInt(QObject* sender);
	void registerButtonClicked(QObject* sender);
	void registerButtonClicked(QObject* sender, unsigned int index);
	void registerItemChanged(QObject* sender);
	void registerEditingFinished(QObject* sender);
	void registerCheckboxToggled(QObject* sender);
	void registerMenuSelected(QObject* sender);
	void registerDeltaChange(QObject* sender, unsigned int index);
	void registerTextureChanged(QObject* sender);
	void registerRayVisButtonSelChanged(QObject* sender);

signals:

public slots:
	void valueChanged();
	void sliderChanged(int value);
	void buttonClicked();
	void buttonClickedIndex();
	void menuSelected(int index);
	void deltaChange(float delta);
	void selectionChanged();

protected:
	Control*	m_pOwner;

	std::map<QObject*, unsigned int>	m_aButtonIndexes;
	std::map<QObject*, unsigned int>	m_aDeltaChangeIndexes;
};

#endif // CONTROL_CONNECTION_PROXY_H
