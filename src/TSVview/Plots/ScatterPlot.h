#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

#include "BasePlot.h"
#include "DataSet.h"

class ScatterPlot
		: public BasePlot
{
	Q_OBJECT

public:
	ScatterPlot(QWidget* parent = 0);
	void setData(DataSet& data, int col1, int col2, QString filename);

private:
	QwtPlotCurve* curve_;
	QwtPlotCurve* curve2_;
	QwtPlotCurve* regression_;
	QwtPlotMarker* regression_marker_;
	QBitArray filter_;
	NumericColumn* col1_;
	NumericColumn* col2_;

private slots:
	void replot_();
};

#endif
