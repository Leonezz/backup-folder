#include "backupinfo.h"

BackupInfo::BackupInfo()
{
}

BackupInfo::BackupInfo(const QString& sourceDirPath
	, const QString& destDirPath
	, const int syncDurationTime)
	: m_taskInfo{sourceDirPath, destDirPath, syncDurationTime}
{	}

BackupInfo::BackupInfo(const BackupInfo& rhs)
{
	this->m_taskInfo._dest = rhs.getDestinationDirPath();
	this->m_taskInfo._source = rhs.getSourceDirPath();
	this->m_taskInfo._duration = rhs.getSyncDurationMinutes();
}

BackupInfo::BackupInfo(const BackupInfo&& rhs)
{
	//copy all data
	this->m_taskInfo._dest = rhs.getDestinationDirPath();
	this->m_taskInfo._source = rhs.getSourceDirPath();
	this->m_taskInfo._duration = rhs.getSyncDurationMinutes();
}

const QByteArray BackupInfo::hash() const
{
	return QCryptographicHash::hash(
		QString(m_taskInfo._source + m_taskInfo._dest).toLocal8Bit()
		, QCryptographicHash::Md5);
}

const QString BackupInfo::getSourceDirPath() const
{
	return this->m_taskInfo._source;
}

const QString BackupInfo::getDestinationDirPath() const
{
	return this->m_taskInfo._dest;
}

const int BackupInfo::getSyncDurationMinutes() const
{
	return this->m_taskInfo._duration;
}

const TaskInfo BackupInfo::getTaskInfo() const
{
	return m_taskInfo;
}

InfoError BackupInfo::selfCheck()
{
	QDir dir;
	dir.cd(m_taskInfo._source);
	if (!dir.exists())
		return InfoError::SourceNotExists;
	dir.cd(m_taskInfo._dest);
	if (!dir.exists())
		return InfoError::DestinationNotExisits;
	//less than 10 minutes
	if (m_taskInfo._duration < 10)
		return SyncDurationTimeShort;
	//longer than 10 days
	if (m_taskInfo._duration > 14400)
		return InfoError::SyncDurationTimeLong;
	//TODO: dest dir spare space check
	return InfoError::AllGood;
}
