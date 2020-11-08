#include "taskcontroller.h"
TaskController::TaskController()
{
	
}

TaskController::~TaskController()
{
	for (auto&& id : m_actuactorMap.keys())
	{
		emit stopTask(id);
		m_actuactorMap.value(id)->deleteLater();
	}
	for (auto&& thread : m_thread)
	{
		thread->quit();
		thread->wait();
		thread->deleteLater();
	}
}

void TaskController::addTask(const TaskInfo& info)
{
	TaskActuator* actuator = new TaskActuator(info);
	const QString taskId = HashTool::md5(info);
	m_actuactorMap.insert(taskId, actuator);
	QObject::connect(this, &TaskController::startTask, actuator, &TaskActuator::start);
	QObject::connect(this, &TaskController::stopTask, actuator, &TaskActuator::terminateTask);
	QObject::connect(actuator, &TaskActuator::updateStatus, this, &TaskController::forwardSyncStatus);
	QThread* thread = new QThread;
	//when actuator terminated,quit the thread
	QObject::connect(actuator, &TaskActuator::terminated
		, [=]() {
			thread->quit();
		});
	//when thread quited,delete the thread and actuator
	QObject::connect(thread, &QThread::finished
		, [=]() {
			thread->deleteLater();
			actuator->deleteLater();
		});
	m_thread.insert(taskId, thread);
	actuator->moveToThread(thread);
	thread->start();
	taskRestart(taskId);
}

void TaskController::deleteTask(const QString& id)
{
	emit stopTask(id);
	m_thread.remove(id);
	m_actuactorMap.remove(id);
}

void TaskController::taskRestart(const QString& id)
{
	emit startTask(id);
}

void TaskController::forwardSyncStatus(const QString& id, const SyncStatus status)
{
	emit syncStatusChanged(id, status);
}