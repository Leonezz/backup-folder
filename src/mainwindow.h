#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "backupinfo.h"
#include "createtask.h"

#define DEBUG
#ifdef DEBUG
#include <QDebug>
#endif // DEBUG


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void getTaskInfo(const QString& sourceDir
        , const QString& destDir
        , const int syncDurationMinutes);
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
    //item model of list view
    QStandardItemModel* m_model;
    //backup task info list
    QList<BackupInfo> m_taskList;
    //list view right key menu
    QMenu* m_rightKeyMenu;
    //create new task dialog
    CreateTask* m_newTaskDialog;
};
#endif // MAINWINDOW_H
