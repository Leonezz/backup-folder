#include "taskactuator.h"

TaskActuator::TaskActuator(const TaskInfo& info)
	: m_info(info)
	, m_timer(new QTimer)
{
	this->initExcept();
	this->initConnections();
	//init work folder when program start
	this->initWorkFolder();
	this->checkDestinationFolder();
}

void TaskActuator::exec()
{
	m_timer->start(this->m_info._duration * 60 * 1000);
}

void TaskActuator::copyFileToDestiantion()
{
	for (auto&& fileToCopy : m_backupList)
	{
		//get dest path
		const QString absDestPath = fileToCopy.replace(m_info._source, m_info._dest);
		//dest dir path is needed when it does not exist
		const QString destDir;
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
		, this, &TaskActuator::checkDestinationFolder);
	//copy when checkover
	QObject::connect(this, &TaskActuator::checkOver
		, this, &TaskActuator::copyFileToDestiantion);
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
	dir.setPath(m_info._source);
	//get file info list of work folder
	const QFileInfoList&& fileInfoList = dir.entryInfoList();
	//push file infos to a stack for a DFS
	QStack<QFileInfo> stack;
	for (auto&& info : fileInfoList)
		stack.push(info);
	QJsonObject root;
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
			object.insert("lastModifiedTime", info.lastModified().toString());
			object.insert("sha-1", HashTool::sha1(info.absoluteFilePath()));
			root.insert(info.absoluteFilePath(), QJsonValue(object));
		}
	}
	QJsonDocument doc(root);
	if (!objectFile.isOpen())
		objectFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	objectFile.write(doc.toJson());
	objectFile.close();
}

void TaskActuator::checkDestinationFolder()
{
	//check and write files path that are being copying to backupList;
	//copy
	emit checkOver();
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
