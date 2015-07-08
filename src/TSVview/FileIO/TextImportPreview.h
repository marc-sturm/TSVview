#ifndef TEXTIMPORTPREVIEW_H
#define TEXTIMPORTPREVIEW_H

#include <QDialog>
#include <QTableWidget>
#include <QTextStream>

#include "ui_TextImportPreview.h"
#include "DataGrid.h"
#include "Parameters.h"

class TextImportPreview
		: public QDialog
{
	Q_OBJECT

public:
	TextImportPreview(QTextStream& stream, QString location, bool csv_mode, QWidget* parent);
	Parameters parameters() const;

public slots:
	void tryImport();

private slots:
	void showFile();

private:
	Ui::TextImportPreview ui_;
	DataGrid* grid_;
	QTextStream& stream_;
	QString location_;
	Parameters params_;
	DataSet data_;
};

#endif
