#include "backupinfo.h"

BackupInfo::BackupInfo(const QString& sourceDirPath
	, const QString& destDirPath
	, const int syncDurationTime)
	: m_sourceDirPath(sourceDirPath)
	, m_destDirPath(destDirPath)
	, m_syncDurationMinutes(syncDurationTime)
{	}

const QString& BackupInfo::toString() const
{
	return this->m_sourceDirPath
		+ "->" + this->m_destDirPath
		+ "(" + QString::number(m_syncDurationMinutes) + ")";
}

const QString& BackupInfo::getSourceDirPath() const
{
	return this->m_sourceDirPath;
}

const QString& BackupInfo::getDestinationDirPath() const
{
	return this->m_destDirPath;
}

const int BackupInfo::getSyncDurationMinutes() const
{
	return this->m_syncDurationMinutes;
}
