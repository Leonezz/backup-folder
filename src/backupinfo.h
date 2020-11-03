#ifndef BACKUPINFO_H
#define BACKUPINFO_H

#include <qobject.h>
#include <QDir>
//info may be invalid
enum InfoError {
    //source dir path does not exist
    SourceNotExists,
    //dest dir path does not exist
    DestinationNotExisits,
    //dest dir spare space not big enough
    DestinationSpaceNotEnough,
    //sync duration time too short
    SyncDurationTimeShort,
    //sync duration time too long
    SyncDurationTimeLong,
    //all good
    AllGood
};
class BackupInfo :
    public QObject
{
public:
    //default constructor is useless
    BackupInfo();
    //
    BackupInfo(const QString& sourceDirPath
        , const QString& destDirPath
        , const int syncDurationMinutes);
    //copy constructor
    BackupInfo(const BackupInfo&);
    BackupInfo(const BackupInfo&&);
    //toString function,use to write log
    const QString toString()const;
    const QString getSourceDirPath()const;
    const QString getDestinationDirPath()const;
    const int getSyncDurationMinutes()const;
    //self check
    InfoError selfCheck();
private:
    //source dir path
    QString m_sourceDirPath;
    //destination dir path
    QString m_destDirPath;
    //sync duration time in minutes
    int m_syncDurationMinutes;
};

Q_DECLARE_METATYPE(BackupInfo)

#endif // BACKUPINFO_H