#ifndef MERGEDIALOG_H
#define MERGEDIALOG_H

#include <QDialog>
#include "ui_MergeDialog.h"

class MergeDialog
		: public QDialog
{
	Q_OBJECT

public:
	MergeDialog(QString header, QWidget* parent = 0);

	QString separator() const;
	QString header() const;

private:
	Ui::MergeDialog ui_;
};

#endif // MERGEDIALOG_H
