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

namespace HashTool {
    static const QByteArray md5(const QString& str)
    {
        return QCryptographicHash::hash(str.toLocal8Bit()
            , QCryptographicHash::Md5);
    }
    static const QByteArray md5(const TaskInfo&& info)
    {
        return md5(info._source + info._dest);
    }
    //calculate sha1 hash of file
    static const QString sha1(const QString& filePath)
    {
        QFile sourceFile(filePath);
        qint64 fileSize = sourceFile.size();
        //read 10240 Bytes max one time
        const qint64 bufSize = 10240;
        if (sourceFile.open(QIODevice::ReadOnly))
        {
            char buffer[bufSize];
            int bytesRead = 0;
            int readSize = qMin(fileSize, bufSize);
            QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
            while (readSize > 0
                && (bytesRead = sourceFile.read(buffer, readSize)) > 0)
            {
                fileSize -= bytesRead;
                sha1Hash.addData(buffer, bytesRead);
                readSize = qMin(fileSize, bufSize);
            }
            sourceFile.close();
            return QString(sha1Hash.result().toHex());
        }
        return {};
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