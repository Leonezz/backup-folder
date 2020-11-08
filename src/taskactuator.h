#pragma once
#include <QObject>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStack>
#include <QTimer>
#include "backupinfo.h"
class TaskActuator : public QObject
{
	Q_OBJECT
public:
	TaskActuator(const TaskInfo& info);
	void exec();
signals:
	void checkOver();
	
private slots:
	//check the dest folder see if the backup is deleted
	void checkDestinationFolder();
	//copy file to backup
	void copyFileToDestiantion();
private:
	//init the except set
	void initExcept();
	//init the connections
	void initConnections();
	//init the config file when new task actuating
	void initWorkFolder();
	//check if folder or file is ignored
	bool isIgnored(const QString& key);
	//read regexp rules from ignore file
	void readIgnoreFile();
private:
	const QString c_configFileFolder = ".backupfolders";
	const QString c_configFileName = "objects.json";
	const QString c_ignoreFileName = ".backupignored";
	QSet<QString> m_except;
	QList<QString> m_backupList;
	const TaskInfo m_info;
	QTimer* m_timer;
};

