#ifndef FilePreview_H
#define FilePreview_H

#include <QDialog>
#include <QTableWidget>
#include "ui_FilePreview.h"
#include "Parameters.h"

class FilePreview
        : public QDialog
{
	Q_OBJECT

public:
    FilePreview(QString filename, QString display_name, QWidget* parent);
	Parameters parameters() const;

public slots:
	void showPreview();

private:
	Ui::FilePreview ui_;
    QString filename_;
    QString display_name_;
};

#endif
