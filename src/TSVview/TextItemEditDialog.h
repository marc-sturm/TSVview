#ifndef TEXTITEMEDITDIALOG_H
#define TEXTITEMEDITDIALOG_H

#include <QWidget>
#include "ui_TextItemEditDialog.h"

class TextItemEditDialog
	: public QDialog
{
	Q_OBJECT

public:
	TextItemEditDialog(QWidget* parent = 0);
	void setText(QString text);
	QString text() const;

private slots:
	void checkText();

private:
	Ui::TextItemEditDialog ui_;
};

#endif // TEXTITEMEDITDIALOG_H
