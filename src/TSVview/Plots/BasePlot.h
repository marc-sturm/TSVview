#ifndef BASEPLOT_H
#define BASEPLOT_H

#include <QWidget>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <qwt_plot.h>

#include "Parameters.h"
#include "ParameterEditor.h"

class BasePlot
		: public QWidget
{
	Q_OBJECT

public:
	BasePlot(QWidget* parent = 0);
	void enableMouseTracking(bool enabled);

public slots:
	void copyToClipboard();
	void saveAsPng();
	void saveAsSvg();


protected:
	Parameters params_;
	QToolBar* toolbar_;
	QwtPlot* plot_;
	ParameterEditor* editor_;
	QLabel* x_label_;
	QLabel* y_label_;
	QString filename_;

	void addToToolbar(QToolButton* button);
	void addSeparatorToToolbar();
	bool eventFilter(QObject* obj, QEvent* event);

protected slots:
	void showSettings_();
};

#endif
