#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_newTaskDialog(new CreateTask(this))
{
    ui->setupUi(this);
    //checking files
    this->initFiles();
    //init context menu
    this->initMenu();
    //connect signal & slots
    this->initConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readTasks()
{
    const QString taskFilePath = c_configPath
        + "/" + c_taskFileName;
    QFile taskFile(taskFilePath);
    //TODO: file open error handle is needed
    taskFile.open(QIODevice::ReadOnly);
    QJsonParseError jsonError;
    //read json arr from file
    QJsonArray jsonTaskArr
        = QJsonDocument::fromJson(taskFile.readAll(),&jsonError).array();
    taskFile.close();
    //TODO: json parse error handle is needed
    QJsonObject jsonObjTask;
    //clear list first
    m_taskList.clear();
    for (auto&& jsonTask : jsonTaskArr)
    {
        jsonObjTask = jsonTask.toObject();
        m_taskList.append({
             jsonObjTask.value("source").toString()
            ,jsonObjTask.value("destination").toString()
            ,jsonObjTask.value("duration").toInt() 
            });
    }
}

void MainWindow::writeTasks()
{
    QJsonArray jsonTaskArr;
    QJsonObject jsonTask;
    //make json arr from m_taskList
    for (auto&& task : m_taskList)
    {
        jsonTask.insert("source", task.getSourceDirPath());
        jsonTask.insert("destination", task.getDestinationDirPath());
        jsonTask.insert("duration", task.getSyncDurationMinutes());
        jsonTaskArr.append(jsonTask);
    }
    const QString taskFilePath = c_configPath
        + "/" + c_taskFileName;
    QFile taskFile(taskFilePath);
    //TODO: add file open error handle
    taskFile.open(QIODevice::ReadWrite);
    taskFile.write(QJsonDocument(jsonTaskArr).toJson());
    taskFile.close();
}

void MainWindow::initFiles()
{
    QDir dir(this->c_configPath);
    //dir check
    if (!dir.exists())
        dir.mkpath(this->c_configPath);
    const QString taskFilePath = c_configPath + "/"
        + c_taskFileName;
    QFile taskFile(taskFilePath);
    //check task files
    if (!taskFile.exists())
    {
        //if task file does not exist,make a new one
        taskFile.open(QIODevice::ReadWrite);
        taskFile.close();
    }
}

void MainWindow::initMenu()
{
    this->m_rightKeyMenu = new QMenu(this);
    //context menu policy set to custom
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    //action call new task edit dialog
    QAction* actionNewItem = new QAction(tr("new Task"));
    //connect the triggered signal of actionNewItem to the createTaskDialog exec
    QObject::connect(actionNewItem, &QAction::triggered,
        [=](bool) {
            m_newTaskDialog->exec();
        });
    m_rightKeyMenu->addAction(actionNewItem);
    //context menu follows the cursor
    QObject::connect(this, &MainWindow::customContextMenuRequested,
        [=](const QPoint& curPos) {
            m_rightKeyMenu->exec(mapToGlobal(curPos));
        });
}

void MainWindow::initConnections()
{
    //get task info from m_newTaskDialog
    QObject::connect(this->m_newTaskDialog, &CreateTask::forwardTaskInfo
        , this, &MainWindow::getTaskInfo);
}

void MainWindow::getTaskInfo(const QString& sourceDir
    , const QString& destDir, const int syncDuration)
{
    BackupInfo info(sourceDir, destDir, syncDuration);
    if (info.selfCheck() == InfoError::AllGood)
    {
        this->m_taskList.append(std::move(info));
        this->writeTasks();
    }
    //TODO: process the error
}