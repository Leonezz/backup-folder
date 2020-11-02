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
public slots:
    void selectSourceDir();
    void selectDestDir();
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