#include "TextItemEditDialog.h"
#include <QPushButton>
#include <QToolTip>

TextItemEditDialog::TextItemEditDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);

	connect(ui_.editor, SIGNAL(textChanged()), this, SLOT(checkText()));
}

void TextItemEditDialog::setText(QString text)
{
	ui_.editor->setPlainText(text);
}

QString TextItemEditDialog::text() const
{
	return ui_.editor->toPlainText();
}

void TextItemEditDialog::checkText()
{
	QString text = ui_.editor->toPlainText();
	QPushButton* ok_btn = ui_.buttons->button(QDialogButtonBox::Ok);
	if (text.contains('\n') || text.contains('\r') || text.contains('\t'))
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
