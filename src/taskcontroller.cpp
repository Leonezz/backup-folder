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
	QObject::connect(this, &TaskController::stopTask, actuator, &TaskActuator::terminate);
	QObject::connect(actuator, &TaskActuator::updateStatus, this, &TaskController::forwardSyncStatus);
	QThread* thread = new QThread;
	m_thread.insert(taskId, thread);
	actuator->moveToThread(thread);
}

void TaskController::taskDeleted(const QString& id)
{
	emit stopTask(id);
	m_thread.value(id)->quit();
	m_thread.value(id)->wait();
	m_thread.remove(id);
	m_actuactorMap.remove(id);
}

void TaskController::taskRefershed(const QString& id)
{
	emit startTask(id);
}

void TaskController::forwardSyncStatus(const QString& id, const SyncStatus status)
{
	emit updateSyncStatus(id, status);
}