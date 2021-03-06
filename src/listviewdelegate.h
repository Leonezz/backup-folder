#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include "backupinfo.h"
#include "syncstatus.h"

class ListViewDelegate : public QStyledItemDelegate
{
public:
	ListViewDelegate(QObject* parent = nullptr);
	//overides
	QWidget* createEditor(QWidget* parent
		, const QStyleOptionViewItem& option
		, const QModelIndex& index)const override;
	void paint(QPainter* painter
		, const QStyleOptionViewItem& option
		, const QModelIndex& index)const override;
	void setEditorData(QWidget* editor
		, const QModelIndex& index) const override;
	void setModelData(QWidget* editor
		, QAbstractItemModel* model
		, const QModelIndex& index) const override;
	QSize sizeHint(const QStyleOptionViewItem& option
		, const QModelIndex& index) const override;
	void updateEditorGeometry(QWidget* editor
		, const QStyleOptionViewItem& option
		, const QModelIndex& index) const override;
};

#endif

