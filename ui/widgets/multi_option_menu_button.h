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

#ifndef MULTI_OPTION_MENU_BUTTON_H
#define MULTI_OPTION_MENU_BUTTON_H

#include <QPushButton>

#include <vector>
#include <string>

class QSignalMapper;
class QAction;

class MultiOptionMenuButton : public QPushButton
{
	Q_OBJECT
public:
	MultiOptionMenuButton(const std::vector<std::string>& options, const std::vector<std::string>* pAlternativeOptions, QWidget* parent = NULL);

	struct SelectedItems
	{
		bool isItemSelected(unsigned int index) const
		{
			unsigned int mask = 1 << index;
			return (selectedBitmask & mask);
		}

		bool			alternative;		// only one for the moment
		unsigned int	selectedBitmask;	// bitmask of standard options in original order
	};

	virtual void mousePressEvent(QMouseEvent* event);

	void setSelectedIndex(unsigned int index);

	SelectedItems getSelectedItems() const;
	bool getEnabledIndexes(std::vector<unsigned int>& indexes);

	void uncheckStandardOptions();
	void uncheckAlternativeOptions();
	void updateTitleFromOptions();

signals:

public slots:
	void menuSelected(int index);


protected:
	bool						m_haveAlternative;
	unsigned int				m_alternativeOffset;
	std::vector<std::string>	m_aOptions;
	QSignalMapper*				m_pSignalMapper;
	std::vector<QAction*>		m_aActions;
};

#endif // MULTI_OPTION_MENU_BUTTON_H
