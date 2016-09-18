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

#include "file_control.h"

#include <QHBoxLayout>
#include <QAction>
#include <QFileDialog>

#include "ui/widgets/push_button_ex.h"
#include "utils/file_helpers.h"

#include "io/file_io_registry.h"

#include "settings.h"

namespace Imagine
{

FileControl::FileControl(const std::string& name, std::string* pairedValue, std::string label, FileCategory category) : Control(name, label),
	m_category(category)
{
	QWidget* mainWidget = new QWidget();

	QHBoxLayout* layout = new QHBoxLayout(mainWidget);
	mainWidget->setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	mainWidget->setMinimumHeight(26);

	m_pLineEdit = new LineEditEx();
	m_pLineEdit->setAcceptDrops(true);

	m_pairedValue = pairedValue;

	m_pLineEdit->setText(pairedValue->c_str());

	m_lastValue = *m_pairedValue;

	m_pConnectionProxy->registerEditingFinished(m_pLineEdit);

	layout->addWidget(m_pLineEdit);

	m_pBrowseButton = new PushButtonEx(QIcon(":/imagine/images/browse.png"), "", mainWidget);
	m_pBrowseButton->setMaximumWidth(20);
	m_pBrowseButton->setMinimumWidth(20);
	m_pBrowseButton->setMaximumHeight(20);
	m_pBrowseButton->setMinimumHeight(20);
	m_pBrowseButton->setToolTip("Browse for a file");

	layout->addSpacing(5);
	layout->addWidget(m_pBrowseButton);

	m_widget = mainWidget;

	m_pConnectionProxy->registerButtonClicked(m_pBrowseButton);
}

FileControl::~FileControl()
{

}

bool FileControl::valueChanged()
{
	*m_pairedValue = m_pLineEdit->text().toStdString();

	if (*m_pairedValue != m_lastValue)
	{
		m_lastValue = *m_pairedValue;
		return true;
	}

	return false;
}

bool FileControl::buttonClicked(unsigned int index)
{
	QFileDialog dialog(m_widget);

	QString defaultDirPath = QDir::homePath();

	std::string fullFilterString = "Files (*.*)";

	std::string specificFileFilter = "";
	std::string specificFilterDesc = "";

	if (m_pairedValue && !m_pairedValue->empty())
	{
		std::string directory = FileHelpers::getFileDirectory(*m_pairedValue);
		if (!directory.empty())
			defaultDirPath = directory.c_str();
		dialog.selectFile((*m_pairedValue).c_str());
	}
	else
	{
		QString newDefaultPathForCategory;
		Settings& settings = Settings::instance();
		// if we've got an empty path, work out what default one to display, based on the category we are
		if (m_category == eTexture)
			newDefaultPathForCategory = settings.getInternal().value("location_settings/texture_path").toString();
		else if (m_category == eEnvironmentMap)
			newDefaultPathForCategory = settings.getInternal().value("location_settings/environment_map_path").toString();
		else if (m_category == eVolumeBuffer)
		{
			newDefaultPathForCategory = settings.getInternal().value("location_settings/volume_buffer_path").toString();
			specificFilterDesc = "Volumes";
			specificFileFilter = FileIORegistry::instance().getQtFileBrowserFilterForRegisteredVolumeReaders();
		}

		if (!newDefaultPathForCategory.isEmpty())
			defaultDirPath = newDefaultPathForCategory;
	}

	dialog.setWindowTitle("Select file");

	if (!specificFilterDesc.empty() && !specificFileFilter.empty())
	{
		fullFilterString = specificFilterDesc + " (" + specificFileFilter + ")";
	}
	dialog.setNameFilter(fullFilterString.c_str());
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setDirectory(defaultDirPath);
	dialog.setViewMode(QFileDialog::Detail);

	if (!dialog.exec())
		 return false;

	QString fileName = dialog.selectedFiles()[0];

	std::string newFilename = fileName.toStdString();
	*m_pairedValue = newFilename;
	refreshFromValue();

	return true;
}

void FileControl::refreshFromValue()
{
	m_pLineEdit->setText(m_pairedValue->c_str());
	m_lastValue = *m_pairedValue;
}

void FileControl::setValue(const std::string& value)
{
	m_pLineEdit->setText(value.c_str());
	m_lastValue = value;
}

std::string FileControl::getValue()
{
	return m_pLineEdit->text().toStdString();
}


} // namespace Imagine
