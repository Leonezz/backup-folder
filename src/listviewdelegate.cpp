#include "listviewdelegate.h"

ListViewDelegate::ListViewDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{
}

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
	if (!index.isValid())
		return;
	painter->save();
	//QStyleOptionViewItem viewOption(option);

	//set item view rect
	QRectF rect = option.rect;
	rect.setWidth(rect.width() - 1);
	rect.setHeight(rect.height() - 1);

	//round angle radius
	//const qreal radius = 7;
	//draw the outline
	QPainterPath path;
	path.moveTo(rect.topRight());
	path.lineTo(rect.topLeft());
	//path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
	path.lineTo(rect.bottomLeft());
	//path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
	path.lineTo(rect.bottomRight());
	//path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
	path.lineTo(rect.topRight());
	//path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

	//render the selected status
	if (option.state.testFlag(QStyle::State_Selected))
	{
		painter->setPen(QPen(Qt::blue));
		painter->setBrush(QColor(229, 241, 255));
		painter->drawPath(path);
	}
	//hover
	else if (option.state.testFlag(QStyle::State_MouseOver))
	{
		painter->setPen(QPen(Qt::green));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(path);
	}
	//normal
	else 
	{
		painter->setPen(QPen(Qt::gray));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(path);
	}

	//source dir path display label rect
	const QRect sourceRect = QRect(rect.left() + 10, rect.top() + 10
		, rect.width() - 30, 20);
	//status color circle label rect
	const QRect statusRect = QRect(sourceRect.right(), rect.top() + 10
		, 10, 10);
	//destination dir path display label rect
	const QRect destRect = QRect(rect.left() + 10, rect.bottom() - 25
		, rect.width() / 2, 20);
	//duration time diaplay label rect
	const QRect durationTimeRect = QRect(rect.right() / 2, rect.bottom() - 25
		, rect.width() / 2, 20);

	//get status
	SyncStatus status = (SyncStatus)(index.data(Qt::UserRole).toInt());
	//get task info
	QVariant variant = index.data(Qt::UserRole + 1);
	BackupInfo info = variant.value<BackupInfo>();

	//sync status map to color of label
	static const QMap<SyncStatus, QColor> map = {
		{SyncStatus::Syncing,QColor(170,170,0)},
		{SyncStatus::Checking,Qt::GlobalColor::yellow},
		{SyncStatus::Finished,Qt::GlobalColor::green},
		{SyncStatus::Failed,Qt::GlobalColor::red}
	};
	painter->setBrush(map[status]);
	painter->setPen(QPen(map[status]));
	
	//draw the status label
	painter->drawEllipse(statusRect);

	//draw source text label
	painter->setPen(QPen(Qt::black));
	painter->setFont(QFont("consolas", 12, QFont::Bold));
	painter->drawText(sourceRect, Qt::AlignLeft, info.getSourceDirPath());
	
	//draw dest text label
	painter->setPen(QPen(Qt::gray));
	painter->setFont(QFont("consolas", 10));
	painter->drawText(destRect, Qt::AlignLeft, info.getDestinationDirPath());

	//draw sync duration time
	const int durationMinutes = info.getSyncDurationMinutes();
	QString durationTime = QString::number(durationMinutes / 60)
		+ "h" + QString::number(durationMinutes % 60) + "m";
	painter->setPen(QPen(Qt::GlobalColor::darkGray));
	painter->setFont(QFont("consolas", 12, QFont::Bold));
	painter->drawText(durationTimeRect, Qt::AlignRight, durationTime);
	painter->restore();
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
	return QSize(option.rect.width(), 60);
}

void ListViewDelegate::updateEditorGeometry(QWidget* editor
	, const QStyleOptionViewItem& option
	, const QModelIndex& index) const
{
}
