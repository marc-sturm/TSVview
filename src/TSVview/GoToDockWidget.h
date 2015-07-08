#ifndef GOTODOCKWIDGET_H
#define GOTODOCKWIDGET_H

#include <QDockWidget>
#include "ui_GoToDockWidget.h"

class GoToDockWidget
		: public QDockWidget
{
	Q_OBJECT

public:
	GoToDockWidget(QWidget* parent = 0);

signals:
	void goToLine(int);

private slots:
	void on_go_button_clicked();
	void valueChanged();
	void keyPressEvent(QKeyEvent* e);
	void focusInEvent(QFocusEvent* e);

private:
	Ui::GoToDockWidget ui_;
};

#endif // GOTODOCKWIDGET_H
