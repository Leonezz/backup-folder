#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDir>

#include "backupinfo.h"

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
private:
    //read tasks form global task file
    void readTasks();
    //write tasks to global task file
    void writeTasks();
    //check the system folder and file
    void initFiles();
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
    //backup info list
    QList<BackupInfo> m_infoList;
};
#endif // MAINWINDOW_H
