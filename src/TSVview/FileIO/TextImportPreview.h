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
    TextImportPreview(QString filename, QString display_name, bool csv_mode, QWidget* parent);
	Parameters parameters() const;

public slots:
	void tryImport();

private slots:
	void showFile();

private:
	Ui::TextImportPreview ui_;
	DataGrid* grid_;
    QString filename_;
    QString display_name_;
	Parameters params_;
	DataSet data_;

    static Parameters defaultParameters();

};

#endif
