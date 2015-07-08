#include <QFileInfo>
#include <QPushButton>
#include <QMenu>
#include <QInputDialog>

#include "XMLImportPreview.h"
#include "DataSet.h"
#include "XMLFile.h"
#include "CustomExceptions.h"
#include "Settings.h"
#include "FilePreview.h"
#include "CustomExceptions.h"

XMLImportPreview::XMLImportPreview(QString filename, QWidget *parent)
	: QDialog(parent)
	, ui_()
	, filename_(filename)
	, params_(XMLFile::defaultParameters())
	, data_()
{
	ui_.setupUi(this);

	setWindowTitle("Preview XML import: " + QFileInfo(filename).fileName());

	grid_ = new DataGrid();
	ui_.gridWidget->layout()->addWidget(grid_);
	grid_->setData(data_, 20);

	connect(ui_.ori_column, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.ori_row, SIGNAL(clicked()), this, SLOT(tryImport()));
	connect(ui_.main_element, SIGNAL(textChanged(const QString&)), this, SLOT(tryImport()));
	connect(ui_.data_element, SIGNAL(textChanged(const QString&)), this, SLOT(tryImport()));
	connect(ui_.data_attribute, SIGNAL(textChanged(const QString&)), this, SLOT(tryImport()));
	connect(ui_.saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
	ui_.ori_row->setChecked(true);

	settings_ = Settings::map("xml_import_settings", QMap<QString,QVariant>());
	updateRestoreButton();

	tryImport();

	connect(ui_.show_file, SIGNAL(clicked()), this, SLOT(showFile()));
}

Parameters XMLImportPreview::parameters() const
{
	return params_;
}

void XMLImportPreview::tryImport()
{
	//update parameters
	params_.setString("main tag", ui_.main_element->text());
	params_.setString("data tag", ui_.data_element->text());
	params_.setString("data attribute", ui_.data_attribute->text());
	if (ui_.ori_column->isChecked())
	{
		params_.setString("orientation", "column");
	}
	else
	{
		params_.setString("orientation", "row");
	}

	//Parse and render data
	try
	{
		data_.blockSignals(true);
		XMLFile::load(data_, filename_, params_);
		data_.blockSignals(false);
		ui_.error_message->setText("");
		grid_->render();
		grid_->resizeColumnsToContents();
		ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
	catch (FileIOException& e)
	{
		ui_.error_message->setText("<font color=red>Error:</font> " + e.message());
		grid_->clearContents();
		ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

void XMLImportPreview::saveSettings()
{
	bool ok = true;
	QString name = QInputDialog::getText(this, "Store xml import settings", "Name:", QLineEdit::Normal, "", &ok);
	if (!ok) return;

	QString serialized = ui_.main_element->text() + ";";
	serialized += ui_.data_element->text() + ";";
	serialized += ui_.data_attribute->text() + ";";
	serialized += ui_.ori_column->isChecked() ? "col" : "row";

	settings_[name] = serialized;
	Settings::setMap("xml_import_settings", settings_);

	updateRestoreButton();
}

void XMLImportPreview::restoreSettings()
{
	QAction* action = qobject_cast<QAction*>(sender());

	QStringList parts = settings_[action->text()].toString().split(';');
	ui_.main_element->setText(parts[0]);
	ui_.data_element->setText(parts[1]);
	ui_.data_attribute->setText(parts[2]);
	if (parts[3]=="col")
	{
		ui_.ori_column->setChecked(true);
	}
	else
	{
		ui_.ori_row->setChecked(true);
	}

	tryImport();
}

void XMLImportPreview::updateRestoreButton()
{
	ui_.restoreButton->setEnabled(false);

	QMenu* menu = new QMenu();
	foreach(QString item, settings_.keys())
	{
		menu->addAction(item, this, SLOT(restoreSettings()));
		ui_.restoreButton->setEnabled(true);
	}
	ui_.restoreButton->setMenu(menu);
}

void XMLImportPreview::showFile()
{
	QFile file(filename_);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		THROW(FileIOException, "Cannot open file '" + filename_ + "'.");
	}

	QTextStream stream(&file);

	FilePreview* dialog = new FilePreview(stream, filename_, this);
	dialog->exec();
}
