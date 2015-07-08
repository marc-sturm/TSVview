#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

#include "BaseColumn.h"
#include "ui_FilterDialog.h"

class FilterDialog
		: public QDialog
{
	Q_OBJECT

public:
	FilterDialog(BaseColumn* column, QWidget *parent = 0);

private slots:
	void set_();
	void addOperation_(Filter::Type type);

	void updateDropdownText();
	void insertSelectedText(QAction* action);

private:
	BaseColumn* column_;
	Ui::FilterDialog ui_;
};

#endif // FILTERDIALOG_H
