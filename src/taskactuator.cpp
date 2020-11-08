#include "taskactuator.h"

TaskActuator::TaskActuator(const TaskInfo& info)
	: m_info(info)
	, m_timer(new QTimer)
{
	this->initExcept();
	this->initConnections();
	//init work folder when program start
	this->initWorkFolder();
	this->checkRepoStatus();
}

void TaskActuator::exec()
{
	m_timer->start(this->m_info._duration * 60 * 1000);
}

void TaskActuator::copyFileToDestiantion()
{
	emit updateStatus(HashTool::md5(m_info), SyncStatus::Syncing);
	for (auto&& fileToCopy : m_backupList)
	{
		//get dest path
		const QString absDestPath = fileToCopy.replace(m_info._source, m_info._dest);
		//dest dir path is needed when it does not exist
		QString destDir;
		const QStringList&& destSplit = absDestPath.split("/");
		for (int i = 0; i < destSplit.length() - 1; ++i)
			destDir += destSplit.at(i);
		//if dest dir does not exist,make one
		QDir dir(destDir);
		if (!dir.exists(destDir))
			dir.mkpath(destDir);
		//if dest file aready exists,remove it
		QFile destFile(absDestPath);
		if (destFile.exists())
			destFile.remove();
		bool ok = QFile::copy(fileToCopy, absDestPath);
	}
	m_backupList.clear();
	emit updateStatus(HashTool::md5(m_info), SyncStatus::Finished);
}

void TaskActuator::deleteFiles()
{
	emit updateStatus(HashTool::md5(m_info), SyncStatus::Syncing);
	for (auto&& fileToDelete : m_deleteList)
	{
		QFile file(fileToDelete);
		if (file.exists())
			file.remove();
	}
	m_deleteList.clear();
	emit updateStatus(HashTool::md5(m_info), SyncStatus::Finished);
}

void TaskActuator::initExcept()
{
	readIgnoreFile();
	m_except.insert(".");
	m_except.insert("..");
	m_except.insert(".backupfolders");
}

void TaskActuator::initConnections()
{
	//start checking when timeout
	QObject::connect(m_timer, &QTimer::timeout
		, this, &TaskActuator::checkRepoStatus);
	//copy when checkover
	QObject::connect(this, &TaskActuator::statusChecked
		, this, &TaskActuator::copyFileToDestiantion);
	//delete when checkover
	QObject::connect(this, &TaskActuator::statusChecked
		, this, &TaskActuator::deleteFiles);
}

void TaskActuator::initWorkFolder()
{
	QDir dir;
	const QString dirAbslotePath = m_info._source + "/" + c_configFileFolder;
	//if config folder not existed
	if (!dir.exists(dirAbslotePath))
		dir.mkpath(dirAbslotePath);
	const QString objectFileAbslotePath = dirAbslotePath + "/" + c_configFileName;
	QFile objectFile(objectFileAbslotePath);
	//make sure the objects file exists
	if (!objectFile.exists())
		objectFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
	QJsonObject root;

	buildFileInfoJson(&root);

	QJsonDocument doc(root);
	if (!objectFile.isOpen())
		objectFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	objectFile.write(doc.toJson());
	objectFile.close();
}

void TaskActuator::checkRepoStatus()
{
	emit updateStatus(HashTool::md5(m_info), SyncStatus::Checking);
	//check and write files path that are being copying to backupList
	const QString objectFilePath = m_info._source + "/"
		+ c_configFileFolder + "/"
		+ c_configFileName;
	QFile objectsFile(objectFilePath);
	if (!objectsFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
	{
		//TODO: add file open error handle
	}
	//build local file tree json
	QJsonObject localRoot;
	buildFileInfoJson(&localRoot);
	//get local file info map
	QVariantMap localMap = localRoot.toVariantMap();

	//read last file tree json
	QJsonDocument doc = QJsonDocument::fromJson(objectsFile.readAll());

	//update objects file
	objectsFile.write(QJsonDocument(localRoot).toJson());

	//last file info map
	QVariantMap lastMap = doc.object().toVariantMap();
	//compare file info between last and local
	for (auto&& path : lastMap.keys())
	{
		if (localMap.contains(path))
		{
			const QVariantMap& lastInfo = lastMap.value(path).toMap();
			const QString& lastModifiedTime = lastInfo.value(c_keyLastModifiedTime).toString();
			const QVariantMap& localInfo = localMap.value(path).toMap();
			const QString& localModifiedTime = localInfo.value(c_keyLastModifiedTime).toString();
			//file modified,compare the sha-1 hash
			if (lastModifiedTime != localModifiedTime)
			{
				//last sha-1 hash
				const QString& lastHash 
					= lastInfo.value(c_keySha1Hash).toString();
				const QString& localHash
					= HashTool::sha1(path);
				//file content changed
				if (lastHash != localHash)
					m_backupList.append(path);
				lastMap.remove(path);
				localMap.remove(path);
			}
		}
		//if path deleted in local,add path to delete list
		else
		{
			m_deleteList.append(path);
			lastMap.remove(path);
		}
	}
	//for files exists in local but does not exist in last,add them to backup list
	for (auto&& addedFile : localMap.keys())
	{
		m_backupList.append(addedFile);
	}
	//copy
	emit statusChecked();
}

bool TaskActuator::isIgnored(const QString& key)
{
	QRegExp reg;
	for (auto&& regStr : m_except)
	{
		reg.setPattern(regStr);
		if (reg.exactMatch(key))
			return true;
	}
	return false;
}

void TaskActuator::readIgnoreFile()
{
	const QString&& ignoreFilePath = m_info._source + "/" 
		+ c_configFileFolder + "/" + c_ignoreFileName;
	QFile ignoreFile(ignoreFilePath);
	if (!ignoreFile.open(QIODevice::ReadOnly))
	{
		ignoreFile.open(QIODevice::WriteOnly);
		ignoreFile.close();
		return;
	}
	while (!ignoreFile.atEnd())
	{
		m_except.insert(ignoreFile.readLine());
	}
	ignoreFile.close();
}

void TaskActuator::buildFileInfoJson(QJsonObject* root)
{
	QDir dir(m_info._source);
	//get file info list of work folder
	const QFileInfoList&& fileInfoList = dir.entryInfoList();
	//push file infos to a stack for a DFS
	QStack<QFileInfo> stack;
	for (auto&& info : fileInfoList)
		stack.push(info);
	QJsonObject object;
	//DFS
	while (!stack.isEmpty())
	{
		const QFileInfo& info = stack.pop();
		if (isIgnored(info.fileName()))
			continue;
		//for dirs, push all its subfolders to stack
		if (info.isDir())
		{
			const QDir subDir(info.absoluteFilePath());
			for (auto&& subInfo : subDir.entryInfoList())
			{
				if (isIgnored(subInfo.fileName()))
					continue;
				stack.push(subInfo);
			}
		}
		//for files,recode its last modified time and sha-1 hash
		else
		{
			object.insert(c_keyLastModifiedTime, info.lastModified().toString());
			object.insert(c_keySha1Hash, HashTool::sha1(info.absoluteFilePath()));
			root->insert(info.absoluteFilePath(), QJsonValue(object));
		}
	}
}
