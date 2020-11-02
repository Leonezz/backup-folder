#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //checking files
    this->initFiles();
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
