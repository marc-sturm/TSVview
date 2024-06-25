#include "TextItemEditDialog.h"
#include <QPushButton>
#include <QToolTip>
#include <QDebug>

TextItemEditDialog::TextItemEditDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);
	ui_.editor->installEventFilter(this);

	connect(ui_.editor, SIGNAL(textChanged()), this, SLOT(checkText()));
	connect(this, SIGNAL(enterPressed()), this, SLOT(acceptIfTextIsValid()));
}

void TextItemEditDialog::setText(QString text)
{
	ui_.editor->setPlainText(text);
}

QString TextItemEditDialog::text() const
{
	return ui_.editor->toPlainText();
}

bool TextItemEditDialog::textIsValid()
{
	QString text = ui_.editor->toPlainText();
	return !text.contains('\n') && !text.contains('\r') && !text.contains('\t');
}

void TextItemEditDialog::checkText()
{
	QPushButton* ok_btn = ui_.buttons->button(QDialogButtonBox::Ok);
	if (!textIsValid())
	{
		if (ok_btn->isEnabled())
		{
			ok_btn->setEnabled(false);
			QPoint pos = ui_.buttons->mapToGlobal(ok_btn->pos());
			QToolTip::showText(pos, "Text contains newline and/or tab characters. It cannot be stored with these characters!");
		}
	}
	else
	{
		ok_btn->setEnabled(true);
	}
}

void TextItemEditDialog::acceptIfTextIsValid()
{
	if (textIsValid()) accept();
}

bool TextItemEditDialog::eventFilter(QObject* obj, QEvent* event)
{

	if (event->type()==QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key()==Qt::Key_Enter || keyEvent->key()==Qt::Key_Return)
		{
			emit enterPressed();
			return true;
		}
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}
