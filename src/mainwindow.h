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

#include "backupinfo.h"
#include "createtask.h"
#include "listviewdelegate.h"

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
public slots:
    //get task info from create task dialog
    void getTaskInfo(const QString& sourceDir
        , const QString& destDir
        , const int syncDurationMinutes);
    //enable filter
    void filterTask();
private:
    //read tasks form global task file
    void readTasks();
    //write all the tasks to global file
    void writeTasks();
    //check the system folder and file
    void initFiles();
    //init list view right key menu and actions
    void initMenu();
    //init all the widget connections
    void initConnections();
    //init the list view include the itemModel,delegate,filterProxyModel
    void initView();
private:
    //software config path in different OS
#ifdef Q_OS_WIN32//Windows
    const QString c_configPath = "C:/\/ProgramData/BackupFolders";
#elif defined Q_OS_LINUX//Linux
    const QString c_configPath = "/usr/share/BackupFolders";
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
    
    //list view right key menu
    QMenu* m_rightKeyMenu;
    //create new task dialog
    CreateTask* m_newTaskDialog;
};
#endif // MAINWINDOW_H
