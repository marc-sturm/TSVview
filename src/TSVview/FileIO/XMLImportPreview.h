#ifndef XMLIMPORTPREVIEW_H
#define XMLIMPORTPREVIEW_H

#include <QDialog>
#include <QTableWidget>

#include "ui_XMLImportPreview.h"
#include "XMLFile.h"
#include "DataGrid.h"

class XMLImportPreview
		: public QDialog
{
	Q_OBJECT

public:
	XMLImportPreview(QString filename, QWidget* parent);
	Parameters parameters() const;

public slots:
	void tryImport();

private slots:
	void showFile();
	void saveSettings();
	void restoreSettings();
	void updateRestoreButton();

private:
	Ui::XMLImportPreview ui_;
	DataGrid* grid_;
	QString filename_;
	Parameters params_;
	DataSet data_;

	QMap<QString, QVariant> settings_;
};

#endif
