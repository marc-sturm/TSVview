#ifndef GREPDIALOG_H
#define GREPDIALOG_H

#include <QDialog>
#include "ui_GrepDialog.h"

class GrepDialog
	: public QDialog
{
	Q_OBJECT

public:
	GrepDialog(QWidget* parent = nullptr);
	QString operation() const;
	QString value() const;
	Qt::CaseSensitivity caseSensitivity() const;

private:
	Ui::GrepDialog ui_;
};

#endif // GREPDIALOG_H
