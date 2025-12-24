#include "FindDockWidget.h"
#include <QKeyEvent>

FindDockWidget::FindDockWidget(QWidget* parent)
	: QDockWidget("GoTo", parent)
	, ui_()
{
	ui_.setupUi(this);

	addOption_(DataGrid::FIND_EXACT);
	addOption_(DataGrid::FIND_CONTAINS);
	addOption_(DataGrid::FIND_START);
	addOption_(DataGrid::FIND_END);
	addOption_(DataGrid::FIND_REGEXP);
	ui_.option->setCurrentIndex(1);

	connect(ui_.edit_field, SIGNAL(returnPressed()), this, SLOT(valueChanged()));
}

void FindDockWidget::on_search_button_clicked()
{
	Qt::CaseSensitivity case_sensitive = Qt::CaseInsensitive;
	if(ui_.case_sensitive->isChecked())
	{
		case_sensitive = Qt::CaseSensitive;
	}

	int index = ui_.option->currentIndex();
	DataGrid::FindType type = (DataGrid::FindType) ui_.option->itemData(index, Qt::UserRole).toInt();

	emit searchForText(ui_.edit_field->text(), case_sensitive, type);
}

void FindDockWidget::on_next_button_clicked()
{
    emit searchNext();
}
void FindDockWidget::keyPressEvent(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Escape && e->modifiers()==Qt::NoModifier)
	{
		this->close();
	}
	else if (e->key()==Qt::Key_F3 && e->modifiers()==Qt::NoModifier)
	{
		emit searchNext();
	}
}

void FindDockWidget::focusInEvent(QFocusEvent* /*event*/)
{
	ui_.edit_field->setFocus();
	ui_.edit_field->selectAll();
}

void FindDockWidget::valueChanged()
{
	on_search_button_clicked();
}

void FindDockWidget::addOption_(DataGrid::FindType type)
{
	ui_.option->addItem(DataGrid::findTypeToString(type), type);
}
