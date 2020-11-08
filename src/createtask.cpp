#include "createtask.h"
#include "ui_createtask.h"

CreateTask::CreateTask(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::Dialog)
    , m_syncDurationMinutes(30)
{
	ui->setupUi(this);
	this->setFixedSize(QSize(400,210));
    const QStringList durationTime = {
        tr("30 minutes"),tr("1 hour"),tr("3 hours"),
        tr("6 hours"),tr("1 Day"),tr("7 Days")
    };
    ui->comboBoxSyncDuration->addItems(durationTime);
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
	m_destDirPath.replace(QDir::separator(), "/");
	m_destDirPath += "/" + m_souceDirPath.split("/").last();
	m_destDirPath.replace("//", "/");
	ui->lineEditDestDir->setText(m_destDirPath);
}

void CreateTask::setSyncDurationTime(int index)
{
    const QMap<int,int> map = {
        {0,30},
        {1,60},
        {2,180},
        {3,360},
        {4,1440},
        {5,10080}
    };
    this->m_syncDurationMinutes = map[index];
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
    QObject::connect(ui->comboBoxSyncDuration, SIGNAL(currentIndexChanged(int))
        , this, SLOT(setSyncDurationTime(int)));
	//task created,emit task info
	QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, [=]() {
		emit forwardTaskInfo(m_souceDirPath, m_destDirPath, m_syncDurationMinutes);
		});
}

void CreateTask::selectSourceDir()
{
	m_souceDirPath = QFileDialog::getExistingDirectory(
		this,
		tr("选择要备份的文件夹"),
		"/"	//select from "/"
	);
	//show the path in the lineEdit
	m_souceDirPath.replace(QDir::separator(), "/");
	ui->lineEditsourceDir->setText(m_souceDirPath);
}
