#ifndef FINDDOCKWIDGET_H
#define FINDDOCKWIDGET_H

#include <QDockWidget>
#include "DataGrid.h"
#include "ui_FindDockWidget.h"

class FindDockWidget
		: public QDockWidget
{
	Q_OBJECT

public:
	FindDockWidget(QWidget* parent = 0);

signals:
	void searchForText(QString text, Qt::CaseSensitivity case_sensitive, DataGrid::FindType type);
	void searchNext();

private slots:
	void on_search_button_clicked();
    void on_next_button_clicked();
	void valueChanged();
	void keyPressEvent(QKeyEvent* e);
	void focusInEvent(QFocusEvent* e);

private:
	Ui::FindDockWidget ui_;

	void addOption_(DataGrid::FindType type);
};

#endif // FINDDOCKWIDGET_H
