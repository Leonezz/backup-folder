#ifndef BACKUPINFO_H
#define BACKUPINFO_H

#include <qobject.h>
class BackupInfo :
    public QObject
{
public:
    BackupInfo() = delete;
    BackupInfo(const QString& sourceDirPath
        , const QString& destDirPath);
    BackupInfo(const BackupInfo&) = delete;
    BackupInfo(const BackupInfo&&) = delete;
    const QString& toString()const;
    const QString& getSourceDirPath()const;
    const QString& getDestinationDirPath()const;
private:
    //source dir path
    QString m_sourceDirPath;
    //destination dir path
    QString m_destDirPath;
};

#endif // BACKUPINFO_H