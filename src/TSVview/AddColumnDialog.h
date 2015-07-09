#ifndef ADDCOLUMNDIALOG_H
#define ADDCOLUMNDIALOG_H

#include <QDialog>

namespace Ui {
class AddColumnDialog;
}

class AddColumnDialog
	: public QDialog
{
	Q_OBJECT

public:
	AddColumnDialog(QStringList names, QWidget* parent = 0);
	~AddColumnDialog();

	QString name();
	int insertBefore();
	QString value();
	bool isFormula();


private:
	Ui::AddColumnDialog *ui;
};

#endif // ADDCOLUMNDIALOG_H
