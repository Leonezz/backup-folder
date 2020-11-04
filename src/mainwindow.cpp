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
    //init view
    this->initView();
    //init context menu
    this->initMenu();
    //connect signal & slots
    this->initConnections();
    //read task data
    this->readTasks();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::filterTask()
{
    //get pressed btn
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());
    static const QMap<QPushButton*, SyncStatus> map = {
        {ui->pushButtonFilterSyncing,SyncStatus::Syncing},
        {ui->pushButtonFilterChecking,SyncStatus::Checking},
        {ui->pushButtonFilterFinished,SyncStatus::Finished},
        {ui->pushButtonFilterFailed,SyncStatus::Failed}
    };
    //get sync status
    SyncStatus type = map[btn];
    bool checked = !btn->isChecked();
    //muti filter set
    static QSet<SyncStatus> filterSet;
    if (checked)
        filterSet.insert(type);
    else 
        filterSet.remove(type);
    //make filter regexp
    QString regStr;
    for (auto&& filterType : filterSet)
        regStr += QString::number(filterType) + "|";
    regStr = regStr.left(regStr.length() - 1);
    m_filterProxyModel->setFilterRegExp(QRegExp(regStr));
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
    //m_taskList.clear();
    for (auto&& jsonTask : jsonTaskArr)
    {
        jsonObjTask = jsonTask.toObject();
        TaskInfo info
        {
             jsonObjTask.value("source").toString()
            ,jsonObjTask.value("destination").toString()
            ,jsonObjTask.value("duration").toInt()
        };
        //m_taskList.append(info);
        //test code
        QStandardItem* item = new QStandardItem;
        item->setData((SyncStatus)(qrand() % 4), Qt::UserRole);
        item->setData(QVariant::fromValue(info), Qt::UserRole + 1);
        item->setEditable(false);
        m_model->appendRow(item);
    }
}

void MainWindow::writeTasks()
{
    QJsonArray jsonTaskArr;
    QJsonObject jsonTask;
    //make json arr from m_taskList
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
    //filter syncing
    QObject::connect(ui->pushButtonFilterSyncing, &QPushButton::pressed
        , this, &MainWindow::filterTask);
    //filter checking
    QObject::connect(ui->pushButtonFilterChecking, &QPushButton::pressed
        , this, &MainWindow::filterTask);
    //filter finished
    QObject::connect(ui->pushButtonFilterFinished, &QPushButton::pressed
        , this, &MainWindow::filterTask);
    //filter failed
    QObject::connect(ui->pushButtonFilterFailed, &QPushButton::pressed
        , this, &MainWindow::filterTask);
}

void MainWindow::initView()
{
    //delegate
    m_delegate = new ListViewDelegate();
    ui->listView->setItemDelegate(m_delegate);
    //set items spacing
    ui->listView->setSpacing(10);
    //set item view mode to icon view
    ui->listView->setViewMode(QListView::ListMode);
    //set enable drag
    ui->listView->setDragEnabled(false);
    //this makes program preforms better
    ui->listView->setUniformItemSizes(true);
    //set item movement static
    ui->listView->setMovement(QListView::Static);
    //set item re-layout automaticly
    ui->listView->setResizeMode(QListView::Adjust);
    //set item re-layout all together
    ui->listView->setLayoutMode(QListView::SinglePass);
    //set filter proxy model
    m_filterProxyModel = new QSortFilterProxyModel(ui->listView);
    m_model = new QStandardItemModel;
    m_filterProxyModel->setSourceModel(m_model);
    m_filterProxyModel->setFilterRole(Qt::UserRole);
    m_filterProxyModel->setDynamicSortFilter(true);

    ui->listView->setModel(m_filterProxyModel);
}

void MainWindow::getTaskInfo(const QString& sourceDir
    , const QString& destDir, const int syncDuration)
{
    BackupInfo info(sourceDir, destDir, syncDuration);
    if (info.selfCheck() == InfoError::AllGood)
    {
        //this->m_taskList.append(std::move(info));
        this->writeTasks();
    }
    //TODO: process the error
}