#include "backupinfo.h"

BackupInfo::BackupInfo(const QString& sourceDirPath, const QString& destDirPath)
	: m_sourceDirPath(sourceDirPath)
	, m_destDirPath(destDirPath)
{	}

const QString& BackupInfo::toString() const
{
	return this->m_sourceDirPath
		+ "->" + this->m_destDirPath;
}

const QString& BackupInfo::getSourceDirPath() const
{
	return this->m_sourceDirPath;
}

const QString& BackupInfo::getDestinationDirPath() const
{
	return this->m_destDirPath;
}
