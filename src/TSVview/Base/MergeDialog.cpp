#include "MergeDialog.h"

MergeDialog::MergeDialog(QString header, QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
	ui_.separator->setFocus();
	ui_.header->setText(header);
}

QString MergeDialog::separator() const
{
	return ui_.separator->text();
}

QString MergeDialog::header() const
{
	return ui_.header->text();
}
