#ifndef BACKUPINFO_H
#define BACKUPINFO_H

#include <qobject.h>
#include <QCryptographicHash>
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
struct TaskInfo
{
    //source dir path
    QString _source;
    //destination dir path
    QString _dest;
    //sync duration time in minutes
    int _duration;
};

namespace TaskInfoHash {
    static const QByteArray md5(const QString& str)
    {
        return QCryptographicHash::hash(str.toLocal8Bit()
            , QCryptographicHash::Md5);
    }
    static const QByteArray md5(const TaskInfo&& info)
    {
        return md5(info._source + info._dest);
    }
}


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
    //calculate Md5 hash
    const QByteArray hash()const;
    const QString getSourceDirPath()const;
    const QString getDestinationDirPath()const;
    const int getSyncDurationMinutes()const;
    const TaskInfo getTaskInfo()const;
    //self check
    InfoError selfCheck();
private:
    TaskInfo m_taskInfo;
};

Q_DECLARE_METATYPE(TaskInfo)

#endif // BACKUPINFO_H