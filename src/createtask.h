#ifndef CREATETASK_H
#define CREATETASK_H
#include <QDialog>
#include <QFileDialog>
QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE
class CreateTask :
    public QDialog
{
    Q_OBJECT
public:
    CreateTask(QWidget* parent = nullptr);
signals:
    //send task info to mainwindow
    void forwardTaskInfo(const QString& sourceDir
        , const QString& destDir
        , const int syncDurationMinutes);
public slots:
    void selectSourceDir();
    void selectDestDir();
    //update m_syncDurationMinutes
    void setSyncDurationTime(int index);
private:
    //init all the connections from buttons to slots
    void initConnections();
private:
    Ui::Dialog* ui;
    //source dir path from lineEdit
    QString m_souceDirPath;
    //destination dir path form lineEdit
    QString m_destDirPath;
    //sync duration time from timeEdit
    int m_syncDurationMinutes;
};
#endif //CREATETASK_H
