#include "createtask.h"
#include "ui_createtask.h"

CreateTask::CreateTask(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::Dialog)
{
	ui->setupUi(this);
	this->setFixedSize(QSize(400,210));
}
