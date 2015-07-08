#ifndef FilePreview_H
#define FilePreview_H

#include <QDialog>
#include <QTableWidget>

#include "ui_FilePreview.h"
#include "XMLFile.h"
#include "DataGrid.h"

class FilePreview
		: public QDialog
{
	Q_OBJECT

public:
	FilePreview(QTextStream& stream, QString location, QWidget* parent);
	Parameters parameters() const;

public slots:
	void showPreview();

private:
	Ui::FilePreview ui_;
	QTextStream& stream_;
	QString location_;
};

#endif
