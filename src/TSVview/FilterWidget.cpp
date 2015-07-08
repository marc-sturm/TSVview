#include "FilterWidget.h"

#include <QMenu>

FilterWidget::FilterWidget(QWidget *parent) :
	QDockWidget(parent)
{
	ui_.setupUi(this);

	connect(ui_.enabled, SIGNAL(toggled(bool)), this, SIGNAL(filterEnabledChanged(bool)));
	connect(ui_.filters, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	connect(ui_.filters, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editFilter(QModelIndex)));
	connect(ui_.reduce, SIGNAL(clicked()), this, SIGNAL(reduceToFiltered()));
	connect(ui_.delete_all, SIGNAL(clicked()), this, SIGNAL(removeAllFilters()));
	connect(ui_.load_set, SIGNAL(clicked()), this, SIGNAL(loadFilter()));
	connect(ui_.store_set, SIGNAL(clicked()), this, SIGNAL(storeFilter()));
	connect(ui_.remove_set, SIGNAL(clicked()), this, SIGNAL(deleteFilter()));
}

void FilterWidget::renderFilters(DataSet& dataset)
{
	//enabled
	ui_.enabled->setChecked(dataset.filtersEnabled());

	//filter list
	ui_.filters->clear();
	for (int c=0; c<dataset.columnCount(); ++c)
	{
		Filter filter = dataset.column(c).filter();
		if (filter.type()!=Filter::NONE)
		{
			QListWidgetItem* item = new QListWidgetItem(filter.asString(dataset.column(c).header(), c));
			item->setData(Qt::UserRole, c);
			ui_.filters->addItem(item);
		}
	}
}

void FilterWidget::contextMenu(QPoint pos)
{
	QListWidgetItem* item = ui_.filters->itemAt(pos);
	if (item == 0)
	{
		return;
	}

	QMenu* menu = new QMenu(this);
	QAction* edit_action = menu->addAction(QIcon(":/Icons/Rename.png"), "Edit");
	QAction* remove_action = menu->addAction(QIcon(":/Icons/Remove.png"), "Remove");

	QAction* action = menu->exec(ui_.filters->viewport()->mapToGlobal(pos));
	if (action==edit_action)
	{
		emit editFilter(item->data(Qt::UserRole).toInt());
	}
	if (action==remove_action)
	{
		emit removeFilter(item->data(Qt::UserRole).toInt());
	}

	delete menu;
}

void FilterWidget::editFilter(QModelIndex index)
{
	emit editFilter(ui_.filters->item(index.row())->data(Qt::UserRole).toInt());
}

