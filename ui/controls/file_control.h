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

#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H

#include "control.h"

#include "ui/widgets/line_edit_ex.h"

#include <QLineEdit>
#include <QPushButton>

class FileControl : public Control
{
public:
	enum FileCategory
	{
		eNormal,
		eTexture,
		eEnvironmentMap,
		eVolumeBuffer
	};

	FileControl(const std::string& name, std::string* pairedValue, std::string label = "", FileCategory category = eNormal);
	virtual ~FileControl();

	virtual bool valueChanged();
	virtual bool buttonClicked(unsigned int index);

	virtual void refreshFromValue();

	void setValue(const std::string& value);
	std::string getValue();

protected:
	FileCategory	m_category;
	LineEditEx*		m_pLineEdit;
	QPushButton*	m_pBrowseButton;
	std::string*	m_pairedValue;

	std::string		m_lastValue;
};

#endif // FILE_CONTROL_H
