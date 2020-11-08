#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_newTaskDialog(new CreateTask(this))
    , m_taskController(new TaskController)
    , m_actionNewItem(new QAction(tr("new task")))
    , m_actionBackupNow(new QAction(tr("backup now")))
    , m_actionModified(new QAction(tr("midify task")))
    , m_actionDelete(new QAction(tr("delete task")))
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

void MainWindow::deleteTasks(const QList<QString>& keys)
{
    for (auto&& taskHash : keys)
    {
        emit deleteATaskById(taskHash);
        m_taskMap.remove(taskHash);
    }
    emit taskMapChanged();
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
        TaskInfo taskInfo{
             jsonObjTask.value("source").toString()
            ,jsonObjTask.value("destination").toString()
            ,jsonObjTask.value("duration").toInt()
        };
        const QString& md5Hash = HashTool::md5(taskInfo);
        if (!m_taskMap.keys().contains(md5Hash))
        {
            m_taskMap[md5Hash] = QPair<TaskInfo, SyncStatus>{
                taskInfo,SyncStatus::Checking
            };
            emit taskInfoColected(std::move(taskInfo));
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
    //QIODevice::Truncate will clear the original file
    taskFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QByteArray str = QJsonDocument(jsonTaskArr).toJson();
    taskFile.write(QJsonDocument(jsonTaskArr).toJson());
    taskFile.close();
}

void MainWindow::updateSyncStatus(const QString& taskId, const SyncStatus status)
{
    QPair<TaskInfo, SyncStatus> pair = m_taskMap.value(taskId);
    pair.second = status;
    m_taskMap.insert(taskId, std::move(pair));
    emit updateListView();
}

void MainWindow::initFiles()
{
    QDir dir(this->c_configPath);
    //dir check
    if (!dir.exists())
        dir.mkpath(this->c_configPath);
    const QString taskFilePath = c_configPath + "/" + c_taskFileName;
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
    //connect the triggered signal of actionNewItem to the createTaskDialog exec
    QObject::connect(m_actionNewItem, &QAction::triggered,
        [=](bool) {
            m_newTaskDialog->exec();
        });
    m_rightKeyMenu->addAction(m_actionNewItem);
    QObject::connect(m_actionBackupNow, &QAction::triggered,
        [=](bool) {
            const QModelIndexList&& selectedItem
                = ui->listView->selectionModel()->selectedIndexes();
            for (auto&& index : selectedItem)
            {
                const QString& taskId 
                    = HashTool::md5(std::move(index.data(Qt::UserRole + 1).value<TaskInfo>()));
                emit startTaskActionTriggered(taskId);
            }
        });
    m_rightKeyMenu->addAction(m_actionBackupNow);

    QObject::connect(m_actionModified, &QAction::triggered,
        [=](bool) {
            //get item that is going to bo modified
            const QModelIndexList &&selectedItem 
                = ui->listView->selectionModel()->selectedIndexes();
            //get taks info
            const TaskInfo &&infoSelectedTask
                = selectedItem.first().data(Qt::UserRole + 1).value<TaskInfo>();
            const QString&& taskHash = HashTool::md5(std::move(infoSelectedTask));
            //remove the original task before call the newTaskDialog
            m_taskMap.remove(taskHash);
            m_newTaskDialog->exec();
        });
    QObject::connect(m_actionDelete, &QAction::triggered,
        [=](bool) {
            QMessageBox::StandardButton btn =
                (QMessageBox::StandardButton)QMessageBox::warning(this
                    , tr("warning")//title
                    , tr("Are you ture to delete selected tasks?")//text
                    , QMessageBox::StandardButton::Yes//btn 1
                    , QMessageBox::StandardButton::No);//btn 2
            if (btn == QMessageBox::StandardButton::Yes)
            {
                //get selected items
                QModelIndexList indexList = ui->listView->selectionModel()->selectedIndexes();
                QList<QString> deleteKeysList;
                for (auto&& index : indexList)
                {
                    //calculate task hash
                    const QString&& md5Hash =
                        HashTool::md5(std::move(index.data(Qt::UserRole + 1).value<TaskInfo>()));
                    deleteKeysList.append(md5Hash);
                }
                deleteTasks(deleteKeysList);
            }
                });
    //context menu follows the cursor
    QObject::connect(this, &MainWindow::customContextMenuRequested,
        [=](const QPoint& curPos) {
            //right key on an item
            m_rightKeyMenu->removeAction(m_actionModified);
            m_rightKeyMenu->removeAction(m_actionDelete);
            QItemSelectionModel* selectionModel = ui->listView->selectionModel();
            if (selectionModel->selectedIndexes().empty())
            {

            }
            else if (selectionModel->selectedIndexes().length() == 1)
            {
                m_rightKeyMenu->addAction(m_actionModified);
                m_rightKeyMenu->addAction(m_actionDelete);
            }
            else 
            {
                m_rightKeyMenu->addAction(m_actionDelete);
            }
            m_rightKeyMenu->exec(mapToGlobal(curPos));
        });
}

void MainWindow::initConnections()
{
    //update list view when tasks info map changed
    QObject::connect(this, &MainWindow::taskMapChanged
        , this, &MainWindow::updateListView);
    //update task file when tasks info map changed
    QObject::connect(this, &MainWindow::taskMapChanged
        , this, &MainWindow::writeTasks);
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
    //add tasks after task info was colected
    QObject::connect(this, &MainWindow::taskInfoColected
        , m_taskController, &TaskController::addTask);
    //delete a task by its id when the delete action was triggered
    QObject::connect(this, &MainWindow::deleteATaskById
        , m_taskController, &TaskController::deleteTask);
    //restart the update task when the restart action was triggered
    QObject::connect(this, &MainWindow::startTaskActionTriggered
        , m_taskController, &TaskController::taskRestart);
    //update the syncstatus to the listview when status changed
    QObject::connect(m_taskController, &TaskController::syncStatusChanged
        , this, &MainWindow::updateSyncStatus);
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

InfoError MainWindow::checkTaskInfo(const TaskInfo& info)
{
    QDir dir;
    //source dir path does not exist
    if (!dir.exists(info._source))
        return InfoError::SourceNotExists;
    //the path that is going to be the backup folder aready exists
    if (dir.exists(info._dest))
        return InfoError::DestinationFolderAdreadyExists;
    QString dest;
    QStringList destList = info._dest.split("/");
    destList.removeLast();
    for (auto&& ch : destList)
        dest += ch + "/";
    //the parent folder of the backup folder does not exist
    if (!dir.exists(dest))
        return InfoError::DestinationNotExisits;
    //duration time less than 10 minutes is too short
    if (info._duration < 10)
        return InfoError::SyncDurationTimeShort;
    //duration time longer than 14400 minutes is too long
    if (info._duration > 14400)
        return InfoError::SyncDurationTimeLong;
    //the destination folder drive need more free space than the source folder size
    if (getDirSize(info._source) > QStorageInfo(info._dest).bytesAvailable())
        return InfoError::DestinationSpaceNotEnough;
    return InfoError::AllGood;
}

quint64 MainWindow::getDirSize(const QString& path)
{
    QDir dir(path);
    quint64 size = 0;
    for (auto&& fileInfo : dir.entryInfoList(QDir::Files))
    {
        size += fileInfo.size();
    }
    for (auto&& subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        size += getDirSize(path + "/" + subDir);
    }
    return size;
}

void MainWindow::getTaskInfo(const QString& sourceDir
    , const QString& destDir, const int syncDuration)
{
    const TaskInfo info{ sourceDir, destDir, syncDuration };
    if (this->checkTaskInfo(info) == InfoError::AllGood)
    {
        const QString& md5Hash = HashTool::md5(info);
        //if new task already exists
        if (m_taskMap.keys().contains(md5Hash))
        {
            //TODO: handle the add same task error
        }
        else 
        {
            m_taskMap[md5Hash] = QPair<TaskInfo, SyncStatus>{
                info ,SyncStatus::Checking
            };
            emit taskMapChanged();
            emit taskInfoColected(info);
        }
    }
    //TODO: process the error
}
