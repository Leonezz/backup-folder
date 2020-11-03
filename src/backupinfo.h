#ifndef BACKUPINFO_H
#define BACKUPINFO_H

#include <qobject.h>
class BackupInfo :
    public QObject
{
public:
    BackupInfo() = delete;
    BackupInfo(const QString& sourceDirPath
        , const QString& destDirPath
        , const int syncDurationMinutes);
    BackupInfo(const BackupInfo&);
    BackupInfo(const BackupInfo&&);
    const QString& toString()const;
    const QString& getSourceDirPath()const;
    const QString& getDestinationDirPath()const;
    const int getSyncDurationMinutes()const;
private:
    //source dir path
    QString m_sourceDirPath;
    //destination dir path
    QString m_destDirPath;
    //sync duration time in minutes
    int m_syncDurationMinutes;
};

#endif // BACKUPINFO_H