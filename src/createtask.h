#ifndef CREATETASK_H
#define CREATETASK_H
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE
class CreateTask :
    public QDialog
{
public:
    CreateTask(QWidget* parent = nullptr);
private:
    Ui::Dialog* ui;
};
#endif //CREATETASK_H
