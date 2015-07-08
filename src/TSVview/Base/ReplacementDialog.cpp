#include "ReplacementDialog.h"
#include "ui_ReplacementDialog.h"

#include <math.h>

#include "QLabel"
#include "QLineEdit"

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
	for (QSet<QString>::const_iterator it=keys.begin(); it!=keys.end(); ++it)
	{
		QLabel* label = new QLabel("'" + *it + "' => ");
		label->setObjectName("l" + QString::number(row));
		layout->addWidget(label, row, 0);

		QLineEdit* edit = new QLineEdit();
		edit->setObjectName("e" + QString::number(row));
		edit->setValidator(new QDoubleValidator(edit));
		layout->addWidget(edit, row, 1);

		++row;
	}

	//store keys
	keys_ = keys;
}

QMap<QString, double> ReplacementDialog::getMap()
{
	QMap<QString, double> output;

	int row = 0;
	for (QSet<QString>::const_iterator it=keys_.begin(); it!=keys_.end(); ++it)
	{
		QLineEdit* edit = ui_.main->findChild<QLineEdit*>("e" + QString::number(row));

		bool ok;
		double value = edit->text().toDouble(&ok);
		if (ok)
		{
			output.insert(*it, value);
		}
		else
		{
			output.insert(*it, NAN);
		}

		++row;
	}

	return output;
}
