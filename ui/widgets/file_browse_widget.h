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

#ifndef FILE_BROWSE_WIDGET_H
#define FILE_BROWSE_WIDGET_H

#include <QWidget>
#include <QPushButton>

#include <string>

#include "ui/widgets/line_edit_ex.h"

class FileBrowseWidget : public QWidget
{
	Q_OBJECT
public:
	// TODO: this is crap...
	FileBrowseWidget(const std::string& path, QWidget *parent = 0);
	FileBrowseWidget(QString& path, QWidget *parent = 0);
	virtual ~FileBrowseWidget();

	void setPath(const std::string& path);
	std::string getPath() const;
	QString getPathQString() const;

	void setDirectoriesOnly(bool dirOnly) { m_directoriesOnly = dirOnly; }
	void setStartInTexturePath(bool startInTexturePath) { m_startInTexturePath = startInTexturePath; }

	void setAcceptTokens(bool acceptTokens) { m_acceptTokens = acceptTokens; }
	void setSave(bool save) { m_save = save; }

	void performBrowse(bool onlyIfEmpty);

protected:
	void pathChanged();

public slots:
	void buttonClicked();
	void textChanged();

signals:
	void pathHasChanged();

protected:
	LineEditEx*		m_pLineEdit;
	QPushButton*	m_pBrowseButton;

	std::string		m_lastPath;

	bool			m_directoriesOnly;
	bool			m_startInTexturePath;

	bool			m_acceptTokens;

	bool			m_save;
};

#endif // FILE_BROWSE_WIDGET_H
