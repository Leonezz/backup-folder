#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStorageInfo>

#include "backupinfo.h"
#include "createtask.h"
#include "listviewdelegate.h"
#include "taskcontroller.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    //emit when filter buttons pressed
    //type tells which button is pressed
    void filterButtonPressed(SyncStatus type
        , bool checked);
    //emit when m_taskMap are modified
    void taskMapChanged();
    void taskInfoColected(const TaskInfo& info);
    void deleteATaskById(const QString& taskId);
    //task start
    void startTaskActionTriggered(const QString& taskid);
public slots:
    //get task info from create task dialog
    void getTaskInfo(const QString& sourceDir
        , const QString& destDir
        , const int syncDurationMinutes);
    //enable filter
    void filterTask();
    //update the list view
    void updateListView();
    //delete task
    void deleteTasks(const QList<QString>& keys);
    //write all the tasks to global file
    void writeTasks();
    //update syncstatus
    void updateSyncStatus(const QString& taskId, const SyncStatus status);
private:
    //read tasks form global task file
    void readTasks();
    //check the system folder and file
    void initFiles();
    //init list view right key menu and actions
    void initMenu();
    //init all the widget connections
    void initConnections();
    //init the list view include the itemModel,delegate,filterProxyModel
    void initView();
    InfoError checkTaskInfo(const TaskInfo& info);
    quint64 getDirSize(const QString& path);
private:
    //software config path in different OS
#ifdef Q_OS_WIN32//Windows
    const QString c_configPath = "C:/ProgramData/BackupFolders";
#elif defined Q_OS_LINUX//Linux
    const QString c_configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.BackupFolders";
#endif
    const QString c_taskFileName = "tasks.json";
    //MainWindow UI
    Ui::MainWindow *ui;
    //delegate to manage viewing items
    ListViewDelegate* m_delegate;
    //item model of list view
    QStandardItemModel* m_model;
    //filter model to filter items
    QSortFilterProxyModel* m_filterProxyModel;
    QMap<QString, QPair<TaskInfo,SyncStatus>> m_taskMap;
    //list view right key menu
    QMenu* m_rightKeyMenu;
    QAction* m_actionNewItem;
    QAction* m_actionBackupNow;
    QAction* m_actionModified;
    QAction* m_actionDelete;
    //create new task dialog
    CreateTask* m_newTaskDialog;
    //brige between tasks and mainwindow
    TaskController* m_taskController;
};
#endif // MAINWINDOW_H
