#include "GrepDialog.h"

GrepDialog::GrepDialog(QWidget* parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);
	ui_.value->setFocus();
}

QString GrepDialog::operation() const
{
	return ui_.operation->currentText();
}

QString GrepDialog::value() const
{
	return ui_.value->text();
}

bool GrepDialog::invert() const
{
	return ui_.invert->isChecked();
}
