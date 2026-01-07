#include <QFileInfo>
#include <QPushButton>
#include "TextImportPreview.h"
#include "DataSet.h"
#include "FilePreview.h"
#include "Exceptions.h"

TextImportPreview::TextImportPreview(QString filename, QString display_name, bool csv_mode, QWidget* parent)
	: QDialog(parent)
	, ui_()
    , filename_(filename)
    , display_name_(display_name)
    , params_(defaultParameters())
	, data_()
{
	ui_.setupUi(this);

    setWindowTitle("Preview text import: " + display_name);

	grid_ = new DataGrid();
	ui_.gridWidget->layout()->addWidget(grid_);
	grid_->setData(data_, 20);

	params_.clearRestrictions();

	if (csv_mode)
	{
		ui_.sep_comma->setChecked(true);
	}
	else
	{
		ui_.sep_tab->setChecked(true);
	}
	connect(ui_.sep_comma, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.sep_custom, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.sep_semicolon, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.sep_space, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.sep_tab, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.separator, SIGNAL(textChanged(const QString&)), this, SLOT(tryImport()));

	ui_.com_hash->setChecked(true);
	connect(ui_.com_none, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.com_hash, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.com_custom, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.comment, SIGNAL(textChanged(const QString&)), this, SLOT(tryImport()));

	ui_.quote_none->setChecked(true);
	connect(ui_.quote_none, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.quote_single, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.quote_double, SIGNAL(clicked()), this, SLOT(tryImport()));

	connect(ui_.first_line_is_comment, SIGNAL(stateChanged(int)), this, SLOT(tryImport()));

	tryImport();

	connect(ui_.show_file, SIGNAL(clicked()), this, SLOT(showFile()));
}

Parameters TextImportPreview::parameters() const
{
	return params_;
}

void TextImportPreview::tryImport()
{
	//Update parameters
	if (ui_.sep_comma->isChecked())
	{
		params_.setChar("separator", ',');
	}
	else if (ui_.sep_semicolon->isChecked())
	{
		params_.setChar("separator", ';');
	}
	else if (ui_.sep_tab->isChecked())
	{
		params_.setChar("separator", '	');
	}
	else if (ui_.sep_space->isChecked())
	{
		params_.setChar("separator", ' ');
	}
	else if (ui_.sep_custom->isChecked())
	{
		QString custom_sep = ui_.separator->text();
		if (custom_sep.length()>0)
		{
			params_.setChar("separator", custom_sep.at(0));
		}
	}

	if (ui_.com_hash->isChecked())
	{
		params_.setChar("comment", '#');
	}
	else if (ui_.com_none->isChecked())
	{
		params_.setChar("comment", QChar::Null);
	}
	else if (ui_.com_custom->isChecked())
	{
		QString custom_com = ui_.comment->text();
		if (custom_com.length()>0)
		{
			params_.setChar("comment", custom_com.at(0));
		}
	}

	if (ui_.quote_none->isChecked())
	{
		params_.setChar("quote", QChar::Null);
	}
	else if (ui_.quote_double->isChecked())
	{
		params_.setChar("quote", '"');
	}
	else if (ui_.quote_single->isChecked())
	{
		params_.setChar("quote", '\'');
	}

	params_.setBool("first_line_is_comment", ui_.first_line_is_comment->isChecked());

	//Parse and render data
	try
    {
		data_.blockSignals(true);
        data_.import(filename_, display_name_, params_, 20);
        data_.blockSignals(false);
        ui_.error_message->setText("");
        grid_->render();
        grid_->resizeColumnsToContents();
        ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    catch (Exception& e)
	{
        ui_.error_message->setText("<font color=red>Error:</font> " + e.message());
        grid_->clearContents();
        ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void TextImportPreview::showFile()
{
    FilePreview* dialog = new FilePreview(filename_, display_name_, this);
	dialog->exec();
}

Parameters TextImportPreview::defaultParameters()
{
    Parameters params;

    QStringList valid_separators;
    valid_separators.append("\t");
    valid_separators.append(" ");
    valid_separators.append(",");
    valid_separators.append(";");
    valid_separators.append("|");
    params.addChar("separator", "", '\t', valid_separators);

    QStringList valid_comments;
    valid_comments.append("#");
    valid_comments.append(";");
    valid_comments.append("");
    params.addChar("comment", "", '#', valid_comments);

    QStringList quotes;
    quotes.append("");
    quotes.append("'");
    quotes.append("\"");
    params.addChar("quote", "", QChar::Null, quotes);
    params.addBool("first_line_is_comment", "", false);

    return params;
}
