#include "ReplacementDialog.h"
#include "ui_ReplacementDialog.h"
#include <math.h>
#include <QDoubleValidator>
#include <QLabel>
#include <QLineEdit>
#include "NumericColumn.h"

ReplacementDialog::ReplacementDialog(QWidget *parent) :
	QDialog(parent)
{
	ui_.setupUi(this);

	setWindowTitle("Replacement table");
}

ReplacementDialog::~ReplacementDialog()
{
}

void ReplacementDialog::setKeys(QSet<QString> keys)
{
	//create layout
	QGridLayout* layout = new QGridLayout();
	ui_.main->setLayout(layout);

	//add labels and line edits
	int row = 0;
	for (QSet<QString>::const_iterator it=keys.cbegin(); it!=keys.cend(); ++it)
	{
		QLabel* label = new QLabel("'" + *it + "' => ");
		label->setObjectName("l" + QString::number(row));
		layout->addWidget(label, row, 0);

		QLineEdit* edit = new QLineEdit();
		edit->setObjectName("e" + QString::number(row));
		QDoubleValidator* validator = new QDoubleValidator(edit);
		validator->setLocale(QLocale::C);
		edit->setValidator(validator);
		layout->addWidget(edit, row, 1);

		++row;
	}

	//store keys
	keys_ = keys;
}

QMap<QString, QPair<double, char> > ReplacementDialog::getMap()
{
	QMap<QString, QPair<double, char>> output;

	int row = 0;
	for (QSet<QString>::const_iterator it=keys_.begin(); it!=keys_.end(); ++it)
	{
		QLineEdit* edit = ui_.main->findChild<QLineEdit*>("e" + QString::number(row));

		output.insert(*it, NumericColumn::toDouble(edit->text(), true));

		++row;
	}

	return output;
}
