#include "taskactuator.h"

TaskActuator::TaskActuator(const TaskInfo& info)
	: m_info(info)
	, m_timer(nullptr)
	, m_stop(false)
	, m_status(SyncStatus::Finished)
	, m_hash(HashTool::md5(info))
{
}

void TaskActuator::start(const QString& hash)
{
	if (m_timer != nullptr)
	{
		m_timer->stop();
		delete m_timer;
	}
	m_timer = new QTimer;
	if (hash != this->m_hash)
		return;
	//if task aready started,ignore the start signal
	if ((this->m_status & SyncStatus::Syncing) || 
		(this->m_status & SyncStatus::Checking))
		return;
	this->initExcept();
	this->initConnections();
	//init work folder when program start
	this->initWorkFolder();
	this->checkRepoStatus();
	m_timer->start(this->m_info._duration * 60 * 1000);
}

void TaskActuator::copyFileToDestiantion()
{
	this->m_status = SyncStatus::Syncing;
	emit updateStatus(m_hash, m_status);
	for (const auto& fileToCopy : m_backupList)
	{
		//stop when needed
		if (m_stop)
			return;
		//get dest path
		QString absDestPath = fileToCopy;
		absDestPath.replace(m_info._source, m_info._dest);
		//dest dir path is needed when it does not exist
		QString destDir;
		QStringList destSplit = absDestPath.split("/");
		destSplit.removeLast();
		for (auto&& ch : destSplit)
			destDir += ch + "/";
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

	this->m_status = SyncStatus::Finished;
	emit updateStatus(m_hash, m_status);
}

void TaskActuator::deleteFiles()
{
	this->m_status = SyncStatus::Syncing;
	emit updateStatus(m_hash, m_status);
	for (auto&& fileToDelete : m_deleteList)
	{		
		//stop when needed
		if (m_stop)
			return;
		QFile file(fileToDelete.replace(m_info._source, m_info._dest));
		if (file.exists())
			file.remove();
	}
	m_deleteList.clear();

	this->m_status = SyncStatus::Finished;
	emit updateStatus(m_hash, m_status);
}

void TaskActuator::terminateTask(const QString& hash)
{
	if (hash != this->m_hash)
		return;
	m_stop = true;
    const QString folderName = m_info._source + "/" + c_configFileFolder;
    QDir dir(folderName);
    const QFileInfoList fileInfos = dir.entryInfoList(QDir::Files | QDir::Hidden);
    for(auto && info : fileInfos)
    {
        QFile file(info.absoluteFilePath());
        if(file.exists())
            file.remove();
    }
    bool d = dir.rmpath(folderName);
	m_timer->stop();
	emit terminated();
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
	{
		objectFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
	}
}

void TaskActuator::checkRepoStatus()
{
	this->m_status = SyncStatus::Checking;
	emit updateStatus(m_hash, m_status);
	//check and write files path that are being copying to backupList
	const QString objectFilePath = m_info._source + "/"
		+ c_configFileFolder + "/"
		+ c_configFileName;
	QFile objectsFile(objectFilePath);
	if (!objectsFile.open(QIODevice::ReadWrite))
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
		//stop when needed
		if (m_stop)
			return;
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
					= HashTool::sha1HashOfFile(path);
				//file content changed
				if (lastHash != localHash)
					m_backupList.append(path);
			}
			lastMap.remove(path);
			localMap.remove(path);
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
		//stop when needed
		if (m_stop)
			return;
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
		//stop when needed
		if (m_stop)
			return;
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
		//stop when needed
		if (m_stop)
			return;
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
			object.insert(c_keySha1Hash, HashTool::sha1HashOfFile(info.absoluteFilePath()));
			root->insert(info.absoluteFilePath(), QJsonValue(object));
		}
	}
}
