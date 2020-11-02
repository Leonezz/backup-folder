#include "createtask.h"
#include "ui_createtask.h"

CreateTask::CreateTask(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::Dialog)
	, m_syncDurationMinutes(10)
{
	ui->setupUi(this);
	this->setFixedSize(QSize(400,210));

	//init all the connections
	initConnections();
}

void CreateTask::selectDestDir()
{
	m_destDirPath = QFileDialog::getExistingDirectory(
		this,
		tr("选择备份文件夹"),
		"/"	//select from "/"
	);
	//show the path in the lineEdit
	ui->lineEditDestDir->setText(m_destDirPath);
}

void CreateTask::setSyncDurationTime()
{
	QTime time = ui->timeEditSyncDuration->time();
	this->m_syncDurationMinutes = time.hour() * 60 + time.minute();
}

void CreateTask::initConnections()
{
	//source
	QObject::connect(ui->pushButtonChooseSourceDir, &QPushButton::clicked
		, this, &CreateTask::selectSourceDir);
	//destination
	QObject::connect(ui->pushButtonChooseDestDir, &QPushButton::clicked
		, this, &CreateTask::selectDestDir);
	//time
	QObject::connect(ui->timeEditSyncDuration, &QTimeEdit::editingFinished
		, this, &CreateTask::setSyncDurationTime);
}

void CreateTask::selectSourceDir()
{
	m_souceDirPath = QFileDialog::getExistingDirectory(
		this,
		tr("选择要备份的文件夹"),
		"/"	//select from "/"
	);
	//show the path in the lineEdit
	ui->lineEditsourceDir->setText(m_souceDirPath);
}
