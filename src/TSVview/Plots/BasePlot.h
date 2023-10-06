#ifndef BASEPLOT_H
#define BASEPLOT_H

#include <QWidget>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include "Parameters.h"
#include "ParameterEditor.h"
#include "MyChartView.h"

class BasePlot
		: public QWidget
{
	Q_OBJECT

public:
	BasePlot(QWidget* parent = 0);
	void enableMouseTracking();
	QAbstractSeries* search(QString name) const;

public slots:
	void copyToClipboard();
	void saveAsPng();
	void saveAsSvg();
	void toggleSeriesVisibility();
	void resetZoom();
	void zoomIn();

protected:
	Parameters params_;
	QHash<QString, int> param_name_to_index_;
	QToolBar* toolbar_;
	MyChartView* chart_view_;
	QChart* chart_;
	ParameterEditor* editor_;
	QLabel* x_label_;
	QLabel* y_label_;
	QLabel* info_label_;
	QString filename_;

	void addToToolbar(QToolButton* button);
	void addSeparatorToToolbar();

protected slots:
	void showSettings();
};

#endif
