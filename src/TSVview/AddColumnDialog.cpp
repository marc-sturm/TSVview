#include "AddColumnDialog.h"
#include "ui_AddColumnDialog.h"

AddColumnDialog::AddColumnDialog(QStringList names, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::AddColumnDialog)
{
	ui->setupUi(this);
	int index = 0;
	foreach(QString name, names)
	{
		ui->pos_col->addItem("[" + QString::number(index) + "] " + name);
		++index;
	}
	ui->pos_col->addItem("[" + QString::number(index) + "] [end]");
	ui->pos_col->setCurrentIndex(index);
}

AddColumnDialog::~AddColumnDialog()
{
	delete ui;
}

QString AddColumnDialog::name()
{
	return ui->name->text();
}

int AddColumnDialog::insertBefore()
{
	return ui->pos_col->currentIndex();
}

QString AddColumnDialog::value()
{
	return ui->value->text();
}

bool AddColumnDialog::isFormula()
{
	return ui->formula->isChecked();
}

int AddColumnDialog::decimals()
{
    return ui->decimals->value();
}
