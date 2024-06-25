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

signals:
	void enterPressed();

private slots:
	void checkText();
	void acceptIfTextIsValid();

private:
	Ui::TextItemEditDialog ui_;
	bool eventFilter(QObject* obj, QEvent* event) override; //event filter for capturing Enter in edit
	bool textIsValid();
};

#endif // TEXTITEMEDITDIALOG_H
