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

#ifndef BACKGROUND_WORKER_THREAD_H
#define BACKGROUND_WORKER_THREAD_H

#include <QThread>

class BackgroundWorkerTask
{
public:
	BackgroundWorkerTask();
    virtual ~BackgroundWorkerTask()
    {
    }

	virtual void doTask() = 0;
};

class BackgroundWorkerUpdateGeometryTask : public BackgroundWorkerTask
{
public:
	BackgroundWorkerUpdateGeometryTask();

	virtual void doTask();
};

class BackgroundWorkerUpdateAllGeometryInstancesForPickingTask : public BackgroundWorkerTask
{
public:
	BackgroundWorkerUpdateAllGeometryInstancesForPickingTask();

	virtual void doTask();
};

class BackgroundWorkerThread : public QThread
{
	Q_OBJECT
public:
	BackgroundWorkerThread(QObject *parent = 0);
	virtual ~BackgroundWorkerThread();

	void performTask(BackgroundWorkerTask* pTask);

signals:

public slots:

protected:
	virtual void run();

protected:
	BackgroundWorkerTask*		m_pTask;
};

#endif // BACKGROUND_WORKER_THREAD_H
