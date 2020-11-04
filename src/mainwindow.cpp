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

void MainWindow::updateListView()
{
    this->m_model->clear();
    for (auto&& task : m_taskMap)
    {
        QStandardItem* item = new QStandardItem;
        item->setData(task.second, Qt::UserRole);
        item->setData(QVariant::fromValue(task.first), Qt::UserRole + 1);
        item->setEditable(false);
        m_model->appendRow(item);
    }
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
    //clear map first
    m_taskMap.clear();
    for (auto&& jsonTask : jsonTaskArr)
    {
        jsonObjTask = jsonTask.toObject();
        BackupInfo bkinfo({
             jsonObjTask.value("source").toString()
            ,jsonObjTask.value("destination").toString()
            ,jsonObjTask.value("duration").toInt()
            });
        QByteArray md5Hash = bkinfo.hash();
        if (!m_taskMap.keys().contains(md5Hash))
        {
            m_taskMap[md5Hash] = QPair<TaskInfo, SyncStatus>{
                bkinfo.getTaskInfo(),SyncStatus::Checking
            };
        }
    }
    //update list view
    emit taskMapChanged();
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
    for (auto&& task : m_taskMap)
    {
        const TaskInfo& info = task.first;
        jsonTask.insert("source", info._source);
        jsonTask.insert("destination", info._dest);
        jsonTask.insert("duration", info._duration);
        jsonTaskArr.append(jsonTask);
    }
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
    //action change the item
    QAction* actionModified = new QAction(tr("change this"));
    //action delete the item
    QAction* actionDelete = new QAction(tr("delete this"));
    //context menu follows the cursor
    QObject::connect(this, &MainWindow::customContextMenuRequested,
        [=](const QPoint& curPos) {
            //right key on an item
            QItemSelectionModel* selectionModel = ui->listView->selectionModel();
            if (selectionModel->selectedIndexes().empty())
            {
                m_rightKeyMenu->removeAction(actionModified);
                m_rightKeyMenu->removeAction(actionDelete);
            }
            else if (selectionModel->selectedIndexes().length() == 1)
            {
                m_rightKeyMenu->addAction(actionModified);
                m_rightKeyMenu->addAction(actionDelete);
            }
            else 
            {
                m_rightKeyMenu->addAction(actionDelete);
            }
            m_rightKeyMenu->exec(mapToGlobal(curPos));
        });
}

void MainWindow::initConnections()
{
    //update list view when tasks info map changed
    QObject::connect(this, &MainWindow::taskMapChanged
        , this, &MainWindow::updateListView);
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
    //set item unselected when list view clicked
    ui->listView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->listView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);

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
        QByteArray md5Hash = info.hash();
        //if new task already exists
        if (m_taskMap.keys().contains(md5Hash))
        {
            //TODO: handle the add same task error
        }
        else 
        {
            m_taskMap[md5Hash] = QPair<TaskInfo, SyncStatus>{
                info.getTaskInfo() ,SyncStatus::Checking
            };
            this->writeTasks();
            emit taskMapChanged();
        }
    }
    //TODO: process the error
}
