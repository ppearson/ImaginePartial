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

#include "line_edit_ex.h"

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QUrl>

#include <QMimeData>

namespace Imagine
{

LineEditEx::LineEditEx(QWidget *parent) : QLineEdit(parent)
{
}

void LineEditEx::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
	{
		event->acceptProposedAction();
	}
}

void LineEditEx::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> urlList = event->mimeData()->urls();

		if (!urlList.isEmpty())
		{
			QString fName;
			QFileInfo info;
			fName = urlList[0].toLocalFile();
			info.setFile(fName);
			if (info.isFile())
			{
				setText(fName);
				emit editingFinished();
			}
		}
	}

	event->acceptProposedAction();
}

} // namespace Imagine
