#include "listviewdelegate.h"

QWidget* ListViewDelegate::createEditor(QWidget* parent
	, const QStyleOptionViewItem& option
	, const QModelIndex& index) const
{
	return nullptr;
}

void ListViewDelegate::paint(QPainter* painter
	, const QStyleOptionViewItem& option
	, const QModelIndex& index) const
{
}

void ListViewDelegate::setEditorData(QWidget* editor
	, const QModelIndex& index) const
{
}

void ListViewDelegate::setModelData(QWidget* editor
	, QAbstractItemModel* model
	, const QModelIndex& index) const
{
}

QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem& option
	, const QModelIndex& index) const
{
	return QSize();
}

void ListViewDelegate::updateEditorGeometry(QWidget* editor
	, const QStyleOptionViewItem& option
	, const QModelIndex& index) const
{
}
