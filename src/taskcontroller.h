#pragma once
#include <qobject.h>
#include <QThread>
#include "taskactuator.h"
class TaskController : public QObject
{
    Q_OBJECT
public:
    TaskController();
    ~TaskController();
    void addTask(const TaskInfo& info);
signals:
    void syncStatusChanged(const QString& id, const SyncStatus status);
    void stopTask(const QString& id);
    void startTask(const QString& id);
public slots:
    void forwardSyncStatus(const QString& id, const SyncStatus status);
    void deleteTask(const QString& id);
    void taskRestart(const QString& id);
private:
    QMap<QString, TaskActuator*> m_actuactorMap;
    QMap<QString, QThread*> m_thread;
};

