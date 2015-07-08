#include "GoToDockWidget.h"
#include <QKeyEvent>

GoToDockWidget::GoToDockWidget(QWidget* parent)
	: QDockWidget("GoTo", parent)
	, ui_()
{
	ui_.setupUi(this);

	connect(ui_.row_box, SIGNAL(editingFinished()), this, SLOT(valueChanged()));
}

void GoToDockWidget::on_go_button_clicked()
{
	emit goToLine(ui_.row_box->value());
}

void GoToDockWidget::keyPressEvent(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Escape && e->modifiers()==Qt::NoModifier)
	{
		this->close();
	}
}

void GoToDockWidget::focusInEvent(QFocusEvent* /*event*/)
{
	ui_.row_box->setFocus();
	ui_.row_box->selectAll();
}

void GoToDockWidget::valueChanged()
{
	// act only if the event was triggered by pressing return
	// not when the focus is lost (e.g. due to ESC pressed)
	if (ui_.row_box->hasFocus())
	{
		on_go_button_clicked();
	}
}
