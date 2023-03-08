#ifndef REPLACEMENTDIALOG_H
#define REPLACEMENTDIALOG_H

#include <QDialog>
#include <QSet>

#include "ui_ReplacementDialog.h"


class ReplacementDialog
		: public QDialog
{
	Q_OBJECT

public:
	ReplacementDialog(QWidget* parent = 0);
	~ReplacementDialog();

	void setKeys(QSet<QString> keys);
	QMap<QString, double> getMap();

private:
	Ui::ReplacementDialog ui_;
	QSet<QString> keys_;
};

#endif // REPLACEMENTDIALOG_H
