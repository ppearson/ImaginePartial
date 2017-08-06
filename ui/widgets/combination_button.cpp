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

#include "combination_button.h"

#include <QMenu>
#include <QSignalMapper>
#include <QStyleOptionButton>
#include <QMouseEvent>

namespace Imagine
{

CombinationButton::CombinationButton(const char** pOptions, QWidget* parent) :	QPushButton(parent)
{
	setFocusPolicy(Qt::NoFocus);

	QMenu* pMenu = new QMenu(this);
	setMenu(pMenu);

	m_pSignalMapper = new QSignalMapper(this);

	unsigned int i = 0;
	while (pOptions[i])
	{
		m_aOptions.push_back(pOptions[i++]);
	}

	// TODO: could do all this in above loop...
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

	pMenu->addSeparator();

	//
	QAction* pSelectAllAction = new QAction("All", this);
	connect(pSelectAllAction, SIGNAL(triggered()), this, SLOT(allSelected()));
	pMenu->addAction(pSelectAllAction);

	QAction* pSelectNoneAction = new QAction("None", this);
	connect(pSelectNoneAction, SIGNAL(triggered()), this, SLOT(noneSelected()));
	pMenu->addAction(pSelectNoneAction);


	connect(m_pSignalMapper, SIGNAL(mapped(int)), this, SLOT(menuSelected(int)));
}

void CombinationButton::mousePressEvent(QMouseEvent* event)
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

unsigned char CombinationButton::getStateBitmask() const
{
	unsigned char finalBitmask = 0;

	for (unsigned int i = 0; i < m_aOptions.size(); i++)
	{
		QAction* pAction = m_aActions[i];

		if (pAction->isChecked())
		{
			unsigned char thisMask = 1 << i;
			finalBitmask |= thisMask;
		}
	}

	return finalBitmask;
}

void CombinationButton::setStateFromBitmask(unsigned char bitmask)
{
	for (unsigned int i = 0; i < m_aOptions.size(); i++)
	{
		QAction* pAction = m_aActions[i];

		unsigned char thisMask = 1 << i;

		if ((bitmask & thisMask))
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

void CombinationButton::menuSelected(int index)
{
	QAction* pSelectedAction = m_aActions[index];

	if (pSelectedAction->isChecked())
	{

	}

	updateTitleFromOptions();
	emit selectionChanged();
}

void CombinationButton::allSelected()
{
	for (unsigned int i = 0; i < m_aOptions.size(); i++)
	{
		QAction* pAction = m_aActions[i];
		pAction->setChecked(true);
	}

	updateTitleFromOptions();
	emit selectionChanged();
}

void CombinationButton::noneSelected()
{
	for (unsigned int i = 0; i < m_aOptions.size(); i++)
	{
		QAction* pAction = m_aActions[i];
		pAction->setChecked(false);
	}

	updateTitleFromOptions();
	emit selectionChanged();
}

void CombinationButton::updateTitleFromOptions()
{
	std::string newTitle;

	// special case all, none, or individual opposite...

	unsigned int actions = m_aActions.size();
	// quick count
	unsigned int itemsChecked = 0;
	for (unsigned int i = 0; i < actions; i++)
	{
		const QAction* pAction = m_aActions[i];
		if (pAction->isChecked())
		{
			itemsChecked ++;
		}
	}

	if (itemsChecked == 0 || itemsChecked == 1 || itemsChecked == (actions - 1) || itemsChecked == (actions - 2) || itemsChecked == actions)
	{
		if (itemsChecked == 0)
		{
			newTitle = "Disabled: All";
		}
		else if (itemsChecked == actions)
		{
			newTitle = "Enabled: All";
		}
		else if (itemsChecked == 1)
		{
			// visible to only one
			newTitle = "Enabled: " + getSelectedList();
		}
		else
		{
			// all but one or two
			newTitle = "Disabled: " + getUnselectedList();
		}

		setText(newTitle.c_str());

		return;
	}

	newTitle = getSelectedList();

	setText(newTitle.c_str());
}

std::string CombinationButton::getSelectedList() const
{
	std::string itemsList;

	unsigned int actions = m_aActions.size();
	for (unsigned int i = 0; i < actions; i++)
	{
		const QAction* pAction = m_aActions[i];
		if (pAction->isChecked())
		{
			const std::string& optionName = m_aOptions[i];
			if (!itemsList.empty())
			{
				itemsList += ", ";
			}

			itemsList += optionName;
		}
	}

	return itemsList;
}

std::string CombinationButton::getUnselectedList() const
{
	std::string itemsList;

	unsigned int actions = m_aActions.size();
	for (unsigned int i = 0; i < actions; i++)
	{
		const QAction* pAction = m_aActions[i];
		if (!pAction->isChecked())
		{
			const std::string& optionName = m_aOptions[i];
			if (!itemsList.empty())
			{
				itemsList += ", ";
			}

			itemsList += optionName;
		}
	}

	return itemsList;
}


} // namespace Imagine
