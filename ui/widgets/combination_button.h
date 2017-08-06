/*
 Imagine
 Copyright 2017 Peter Pearson.

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

#ifndef COMBINATION_BUTTON_H
#define COMBINATION_BUTTON_H

#include <QPushButton>

#include <vector>
#include <string>

class QSignalMapper;
class QAction;

namespace Imagine
{

class CombinationButton : public QPushButton
{
	Q_OBJECT
public:
	CombinationButton(const char** pOptions, QWidget* parent = NULL);

	virtual void mousePressEvent(QMouseEvent* event);

	unsigned char getStateBitmask() const;
	void setStateFromBitmask(unsigned char bitmask);

protected:
	void updateTitleFromOptions();

	std::string getSelectedList() const;
	std::string getUnselectedList() const;

signals:
	void selectionChanged();

public slots:
	void menuSelected(int index);

	void allSelected();
	void noneSelected();

protected:
	std::vector<std::string>	m_aOptions;
	QSignalMapper*				m_pSignalMapper;
	std::vector<QAction*>		m_aActions;
};

} // namespace Imagine

#endif // COMBINATION_BUTTON_H
