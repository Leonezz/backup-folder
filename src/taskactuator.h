#pragma once
#include <QObject>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStack>
#include <QTimer>
#include "backupinfo.h"
#include "syncstatus.h"
class TaskActuator : public QObject
{
	Q_OBJECT
public:
	TaskActuator(const TaskInfo& info);
	void exec();
signals:
	void statusChecked();
	void updateStatus(const QString& taskHash, const SyncStatus status);
private slots:
	//check the dest folder see if the backup is deleted
	void checkRepoStatus();
	//copy file to backup
	void copyFileToDestiantion();
	//delete files of local
	void deleteFiles();
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
	//read file tree and build a json object
	void buildFileInfoJson(QJsonObject* root);
private:
	const QString c_configFileFolder = ".backupfolders";
	const QString c_configFileName = "objects.json";
	const QString c_ignoreFileName = ".backupignored";
	const QString c_keyLastModifiedTime = "lastModifiedTime";
	const QString c_keySha1Hash = "sha-1";
	QSet<QString> m_except;
	QList<QString> m_backupList;
	QList<QString> m_deleteList;
	const TaskInfo m_info;
	QTimer* m_timer;
};

