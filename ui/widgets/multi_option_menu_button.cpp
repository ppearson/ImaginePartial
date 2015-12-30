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

#include "multi_option_menu_button.h"

#include <QMenu>
#include <QSignalMapper>
#include <QStyleOptionButton>
#include <QMouseEvent>

MultiOptionMenuButton::MultiOptionMenuButton(const std::vector<std::string>& options, const std::vector<std::string>* pAlternativeOptions, QWidget* parent)
	: QPushButton(parent), m_aOptions(options)
{
	setFocusPolicy(Qt::NoFocus);

	QMenu* pMenu = new QMenu(this);
	setMenu(pMenu);

	m_pSignalMapper = new QSignalMapper(this);

	unsigned int menuIndex = 0;
	std::vector<std::string>::const_iterator it = m_aOptions.begin();
	for (; it != m_aOptions.end(); ++it)
	{
		const std::string& optionName = *it;
		QAction* pNewOption = new QAction(optionName.c_str(), this);
		connect(pNewOption, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
		m_pSignalMapper->setMapping(pNewOption, menuIndex);

		pNewOption->setCheckable(true);
		m_aActions.push_back(pNewOption);

		pMenu->addAction(pNewOption);

		menuIndex ++;
	}

	if (pAlternativeOptions)
	{
		m_haveAlternative = true;
		m_alternativeOffset = menuIndex;

		pMenu->addSeparator();

		it = pAlternativeOptions->begin();
		for (; it != pAlternativeOptions->end(); ++it)
		{
			const std::string& optionName = *it;
			QAction* pNewOption = new QAction(optionName.c_str(), this);
			connect(pNewOption, SIGNAL(triggered()), m_pSignalMapper, SLOT(map()));
			m_pSignalMapper->setMapping(pNewOption, menuIndex);

			m_aOptions.push_back(optionName);

			pNewOption->setCheckable(true);
			m_aActions.push_back(pNewOption);

			pMenu->addAction(pNewOption);

			menuIndex ++;
		}
	}

	connect(m_pSignalMapper, SIGNAL(mapped(int)), this, SLOT(menuSelected(int)));
}

void MultiOptionMenuButton::mousePressEvent(QMouseEvent* event)
{
	QStyleOptionButton style_option;
	style_option.init(this);
	QRect rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &style_option, this);
	QRect popupArea(rect);

	if (popupArea.contains(event->pos()))
	{
		QMenu* pMenu = menu();
		QSize menuSize = pMenu->sizeHint();

		QSize buttonSize = geometry().size();

		pMenu->move(mapToGlobal(QPoint(buttonSize.width() - menuSize.width(), buttonSize.height())));
		pMenu->show();
	}
}

void MultiOptionMenuButton::setSelectedIndex(unsigned int index)
{
	unsigned int actions = m_aActions.size();
	for (unsigned int i = 0; i < actions; i++)
	{
		QAction* pAction = m_aActions[i];
		if (index == i)
		{
			pAction->setChecked(true);
		}
		else
		{
			pAction->setChecked(false);
		}
	}

	updateTitleFromOptions();
}

MultiOptionMenuButton::SelectedItems MultiOptionMenuButton::getSelectedItems() const
{
	SelectedItems items;
	if (m_haveAlternative)
	{
		// see if alternative item is checked
		QAction* pAlternative = m_aActions[m_alternativeOffset];
		if (pAlternative->isChecked())
		{
			items.alternative = true;
			items.selectedBitmask = 0;

			return items;
		}
	}

	// otherwise, it must be a standard item...
	items.alternative = false;

	unsigned int finalMask = 0;

	unsigned int numItems = (m_haveAlternative) ? m_alternativeOffset : m_aOptions.size();
	for (unsigned int i = 0; i < numItems; i++)
	{
		QAction* pAction = m_aActions[i];

		if (pAction->isChecked())
		{
			unsigned int thisMask = 1 << i;
			finalMask |= thisMask;
		}
	}

	items.selectedBitmask = finalMask;

	return items;
}

bool MultiOptionMenuButton::getEnabledIndexes(std::vector<unsigned int>& indexes)
{
	// doesn't take account of alternative items...

	unsigned int actions = m_aActions.size();
	for (unsigned int i = 0; i < actions; i++)
	{
		const QAction* pAction = m_aActions[i];
		if (pAction->isChecked())
		{
			indexes.push_back(i);
		}
	}

	return !indexes.empty();
}

void MultiOptionMenuButton::uncheckStandardOptions()
{
	unsigned int numItems = (m_haveAlternative) ? m_alternativeOffset : m_aOptions.size();
	for (unsigned int i = 0; i < numItems; i++)
	{
		QAction* pAction = m_aActions[i];

		pAction->setChecked(false);
	}
}

void MultiOptionMenuButton::uncheckAlternativeOptions()
{
	if (!m_haveAlternative && m_alternativeOffset > 0)
		return;

	for (unsigned int i = m_alternativeOffset; i < m_aOptions.size(); i++)
	{
		QAction* pAction = m_aActions[i];

		pAction->setChecked(false);
	}
}

void MultiOptionMenuButton::updateTitleFromOptions()
{
	std::string newTitle;

	unsigned int actions = m_aActions.size();
	for (unsigned int i = 0; i < actions; i++)
	{
		const QAction* pAction = m_aActions[i];
		if (pAction->isChecked())
		{
			const std::string& optionName = m_aOptions[i];
			if (!newTitle.empty())
			{
				newTitle += ", ";
			}

			newTitle += optionName;
		}
	}

	setText(newTitle.c_str());
}

void MultiOptionMenuButton::menuSelected(int index)
{
	if (index < m_alternativeOffset)
	{
		// normal options
		uncheckAlternativeOptions();
	}
	else
	{
		// alternative
		uncheckStandardOptions();
	}

	QAction* pSelectedAction = m_aActions[index];

	if (pSelectedAction->isChecked())
	{

	}

	updateTitleFromOptions();
}
