/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#include "file_browse_widget.h"

#include <QHBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QApplication>

#include "global_context.h"

#include "ui/widgets/push_button_ex.h"
#include "utils/file_helpers.h"

#include "settings.h"

FileBrowseWidget::FileBrowseWidget(const std::string& path, QWidget *parent) : QWidget(parent), m_pLineEdit(NULL), m_pBrowseButton(NULL),
	m_directoriesOnly(true), m_acceptTokens(false), m_save(false)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	m_pLineEdit = new LineEditEx();
	m_pLineEdit->setAcceptDrops(true);
	m_pLineEdit->setText(path.c_str());

	layout->addWidget(m_pLineEdit);

	m_pBrowseButton = new PushButtonEx(QIcon(":/imagine/images/browse.png"), "", this);
	m_pBrowseButton->setMaximumWidth(20);
	m_pBrowseButton->setMinimumWidth(20);
	m_pBrowseButton->setMaximumHeight(20);
	m_pBrowseButton->setMinimumHeight(20);
	m_pBrowseButton->setToolTip("Browse for a file");

	layout->addSpacing(8);
	layout->addWidget(m_pBrowseButton);

	QObject::connect(m_pBrowseButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
	QObject::connect(m_pLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
}

FileBrowseWidget::FileBrowseWidget(QString& path, QWidget *parent) : QWidget(parent), m_pLineEdit(NULL), m_pBrowseButton(NULL),
	m_acceptTokens(false), m_save(false)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	setLayout(layout);
	layout->setSpacing(1);
	layout->setMargin(0);

	m_pLineEdit = new LineEditEx();
	m_pLineEdit->setAcceptDrops(true);
	m_pLineEdit->setText(path);

	layout->addWidget(m_pLineEdit);

	m_pBrowseButton = new PushButtonEx(QIcon(":/imagine/images/browse.png"), "", this);
	m_pBrowseButton->setMaximumWidth(20);
	m_pBrowseButton->setMinimumWidth(20);
	m_pBrowseButton->setMaximumHeight(20);
	m_pBrowseButton->setMinimumHeight(20);
	m_pBrowseButton->setToolTip("Browse for a file");

	layout->addSpacing(5);
	layout->addWidget(m_pBrowseButton);

	QObject::connect(m_pBrowseButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

FileBrowseWidget::~FileBrowseWidget()
{
	if (m_pLineEdit)
	{
		delete m_pLineEdit;
		m_pLineEdit = NULL;
	}

	if (m_pBrowseButton)
	{
		delete m_pBrowseButton;
		m_pBrowseButton = NULL;
	}
}

void FileBrowseWidget::setPath(const std::string& path)
{
	m_pLineEdit->setText(path.c_str());
}

std::string FileBrowseWidget::getPath() const
{
	return m_pLineEdit->text().toStdString();
}

QString FileBrowseWidget::getPathQString() const
{
	return m_pLineEdit->text();
}

void FileBrowseWidget::performBrowse(bool onlyIfEmpty)
{
	if (onlyIfEmpty)
	{
		if (!m_pLineEdit->text().isEmpty())
			return;

		// if shift is held down, don't bother either...
		if (QApplication::keyboardModifiers() & Qt::SHIFT)
		{
			// but set focus to line edit
			m_pLineEdit->setFocus();

			return;
		}
	}

	QFileDialog dialog(this);

	Settings& settings = Settings::instance();
	QString dirPath;

	GlobalContext& gc = GlobalContext::instance();

	if (gc.shouldRememberLastTexturePath())
	{
		dirPath = gc.getLastTexturePath().c_str();
	}

	if (dirPath.isEmpty() && gc.isMaterialEditorActive())
	{
		dirPath = gc.getMaterialEditorSessionTexturePath().c_str();
	}

	if (dirPath.isEmpty())
	{
		dirPath = settings.getInternal().value("location_settings/texture_path").toString();
	}

	if (!m_pLineEdit->text().isEmpty())
		dirPath = m_pLineEdit->text();

	dialog.setWindowTitle("Select File");
	dialog.setNameFilter(QWidget::tr("Files (*.*)"));
	if (m_directoriesOnly)
	{
		dialog.setWindowTitle("Select Dir");
		dialog.setFileMode(QFileDialog::DirectoryOnly);
	}
	else
	{
		if (m_acceptTokens)
		{
			dialog.setFileMode(QFileDialog::ExistingFiles);
		}
	}
	dialog.setDirectory(dirPath);
	dialog.setViewMode(QFileDialog::Detail);
	if (m_save)
	{
		dialog.setAcceptMode(QFileDialog::AcceptSave);
	}

	if (!dialog.exec())
		 return;

	std::string newPath;
	std::string currentDirectory;

	if (!m_acceptTokens || dialog.selectedFiles().size() == 1)
	{
		QString fileName = dialog.selectedFiles()[0];

		m_pLineEdit->setText(fileName);

		newPath = fileName.toStdString();
		currentDirectory = FileHelpers::getFileDirectory(newPath);
	}
	else
	{
		QStringList selectedFiles = dialog.selectedFiles();
		unsigned int numStrings = selectedFiles.size();

		bool error = false;
		unsigned int commonPrefixCount = 0;
		unsigned int digitCount = 0;
		unsigned int commonSuffixCount = 0;
		bool donePrefix = false;
		QString firstFileName = selectedFiles[0];
		unsigned int filenameLength = firstFileName.size();

		// look for 4 consecutive numbers in the first string...
		unsigned int digitStartPos = 0;
		unsigned int lastDigitPos = 0;
		unsigned int consecutiveDigitCount = 0;
		for (unsigned int i = 0; i < filenameLength; i++)
		{
			QChar testChar = firstFileName[i];
			char tC = testChar.toAscii();
			bool isDigit = testChar.isDigit();

			if (!isDigit && consecutiveDigitCount == 0)
			{
				digitStartPos ++;
			}
			else if (isDigit && !testChar.isPunct())
			{
				if (lastDigitPos == i - 1)
				{
					consecutiveDigitCount ++;
				}
				lastDigitPos = i;
			}
		}

		if (consecutiveDigitCount == 3 && lastDigitPos != 0)
		{
			// it's very likely a UDIM...
		}
/*
		for (unsigned int testPos = 0; testPos < filenameLength; testPos++)
		{
			bool isDigit = firstFileName[testPos].isDigit();
			QChar compareChar = firstFileName[testPos];
			bool sameDigit = true;

			for (unsigned int i = 1; i < numStrings; i++)
			{
				QString testFileName = selectedFiles[i];

				if (testFileName.size() != filenameLength)
				{
					error = true;
					break;
				}

				if (testFileName[testPos] != firstFileName[testPos])
				{
					// see if it's a number...
					QChar itemChar = testFileName[testPos];

					if (!itemChar.isDigit())
					{
						error = true;
						break;
					}
					else
					{
						// see if it's the same as the others
						if (itemChar != compareChar)
						{
							sameDigit = false;
						}
					}
				}
			}

			if (error)
			{
				break;
			}

			if (isDigit && !sameDigit)
			{
				donePrefix = true;
				digitCount ++;
			}

			if (donePrefix)
			{
				commonSuffixCount ++;
			}
			else
			{
				commonPrefixCount ++;
			}
		}

*/		int one = 5;
	}

	if (gc.shouldRememberLastTexturePath())
	{
		gc.setLastTexturePath(currentDirectory);
	}

	if (gc.isMaterialEditorActive())
	{
		gc.setMaterialEditorSessionTexturePath(currentDirectory);
	}

	if (newPath != m_lastPath)
	{
		m_lastPath = newPath;
		emit pathHasChanged();
	}
}

void FileBrowseWidget::buttonClicked()
{
	performBrowse(false);
}

void FileBrowseWidget::textChanged()
{
	QString newTextStr = m_pLineEdit->text();
	std::string newPath = newTextStr.toStdString();

	if (newPath != m_lastPath)
	{
		m_lastPath = newPath;
		emit pathHasChanged();
	}
}
