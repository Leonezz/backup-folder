#include "backupinfo.h"

BackupInfo::BackupInfo(const QString& sourceDirPath
	, const QString& destDirPath
	, const int syncDurationTime)
	: m_sourceDirPath(sourceDirPath)
	, m_destDirPath(destDirPath)
	, m_syncDurationMinutes(syncDurationTime)
{	}

BackupInfo::BackupInfo(const BackupInfo& rhs)
{
	this->m_destDirPath = rhs.getDestinationDirPath();
	this->m_sourceDirPath = rhs.getSourceDirPath();
	this->m_syncDurationMinutes = rhs.getSyncDurationMinutes();
}

BackupInfo::BackupInfo(const BackupInfo&& rhs)
{
	//copy all data
	this->m_destDirPath = rhs.getDestinationDirPath();
	this->m_sourceDirPath = rhs.getSourceDirPath();
	this->m_syncDurationMinutes = rhs.getSyncDurationMinutes();
}

const QString BackupInfo::toString() const
{
	return this->m_sourceDirPath
		+ "->" + this->m_destDirPath
		+ "(" + QString::number(m_syncDurationMinutes) + ")";
}

const QString BackupInfo::getSourceDirPath() const
{
	return this->m_sourceDirPath;
}

const QString BackupInfo::getDestinationDirPath() const
{
	return this->m_destDirPath;
}

const int BackupInfo::getSyncDurationMinutes() const
{
	return this->m_syncDurationMinutes;
}

InfoError BackupInfo::selfCheck()
{
	QDir dir;
	dir.cd(m_sourceDirPath);
	if (!dir.exists())
		return InfoError::SourceNotExists;
	dir.cd(m_destDirPath);
	if (!dir.exists())
		return InfoError::DestinationNotExisits;
	//less than 10 minutes
	if (m_syncDurationMinutes < 10)
		return SyncDurationTimeShort;
	//longer than 10 days
	if (m_syncDurationMinutes > 14400)
		return InfoError::SyncDurationTimeLong;
	//TODO: dest dir spare space check
	return InfoError::AllGood;
}
