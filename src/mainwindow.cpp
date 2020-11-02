#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //checking files
    this->initFiles();
    //init context menu
    this->initMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    qDebug() << this->c_configPath;
}

void MainWindow::initMenu()
{
    this->m_rightKeyMenu = new QMenu(this);
    //context menu policy set to custom
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    //action call new task edit dialog
    QAction* actionNewItem = new QAction("new Task");
    m_rightKeyMenu->addAction(actionNewItem);
    //context menu follows the cursor
    QObject::connect(this, &MainWindow::customContextMenuRequested,
        [=](const QPoint& curPos) {
            m_rightKeyMenu->exec(mapToGlobal(curPos));
        });
}