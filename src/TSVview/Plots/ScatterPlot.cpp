#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <QDebug>

#include "ScatterPlot.h"
#include "StatisticsSummary.h"
#include "BasicStatistics.h"

ScatterPlot::ScatterPlot(QWidget *parent)
	: BasePlot(parent)
	, curve_(0)
	, curve2_(0)
	, regression_(0)
	, regression_marker_(0)
{
	//set default parameters
	params_.addColor("color", "", Qt::darkBlue);
	params_.addInt("line width", "", 2, 1, 999);
	params_.addSymbol("symbol", "", QwtSymbol::XCross);
	params_.addInt("symbol size", "", 5, 2, 999);
	params_.addSeparator();
	params_.addBool("linear regression", "Show linear regression of unfiltered data.", false);
	params_.addSeparator();
	params_.addBool("filtered", "Show filtered-out data points.", false);
	params_.addColor("filtered color", "", QColor(200, 0, 0));

	//connect parameters, editor and plot
	editor_->setParameters(params_);
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(replot_()));

	//enable mouse tracking
	enableMouseTracking(true);
}

void ScatterPlot::setData(DataSet& data, int col1, int col2, QString filename)
{
	filter_ = data.getRowFilter(false);
	col1_ = &(data.numericColumn(col1));
	col2_ = &(data.numericColumn(col2));
	filename_ = filename;

	plot_->setAxisTitle(QwtPlot::xBottom, QwtText(col1_->headerOrIndex(col1)));
	plot_->setAxisTitle(QwtPlot::yLeft, QwtText(col2_->headerOrIndex(col2)));

	//create curve (except for filtered-out data)
	delete curve_;
	curve_ = new QwtPlotCurve();
	curve_->setStyle(QwtPlotCurve::NoCurve);

	QVector<double> x = col1_->values(filter_);
	QVector<double> y = col2_->values(filter_);
	curve_->setSamples(x,y);
	curve_->setTitle("passing filter");
	curve_->attach(plot_);

	//create curve (filtered-out data)
	QBitArray filter2 = ~filter_;
	delete curve2_;
	curve2_ = new QwtPlotCurve();
	curve2_->setStyle(QwtPlotCurve::NoCurve);
	QVector<double> x2 = col1_->values(filter2);
	QVector<double> y2 = col2_->values(filter2);
	curve2_->setSamples(x2, y2);
	curve2_->setTitle("not passing filter");
	curve2_->attach(plot_);

	//calculate linear regression
	QPair<double, double> reg = BasicStatistics::linearRegression(x, y);
	double offset = reg.first;
	double slope = reg.second;

	// calcualte R-squared
	StatisticsSummary x_stats = col1_->statistics(filter_);
	StatisticsSummary y_stats = col2_->statistics(filter_);
	double mean = y_stats.mean;
	double data_diff = 0.0;
	double model_diff = 0.0;
	for (int i=0; i<y.size(); ++i)
	{
		if (BasicStatistics::isValidFloat(x[i]) && BasicStatistics::isValidFloat(y[i]))
		{
			model_diff += pow(offset + slope * x[i] - mean, 2.0);
			data_diff += pow(y[i] - mean, 2.0);
		}
	}
	double r_squared = model_diff / data_diff;

	//create linear regression plot curve
	delete regression_;
	regression_ = new QwtPlotCurve();
	x.clear();
	x.append(x_stats.min);
	x.append(x_stats.max);
	y.clear();
	y.append(offset + slope * x_stats.min);
	y.append(offset + slope * x_stats.max);
	regression_->setSamples(x, y);
	regression_->attach(plot_);

	// create regression marker
	delete regression_marker_;
	regression_marker_ = new QwtPlotMarker();
	regression_marker_->setXValue(x_stats.max);
	regression_marker_->setYValue(y_stats.min);
	regression_marker_->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	regression_marker_->setLabel(QwtText("R² = " + QString::number(r_squared)));
	regression_marker_->attach(plot_);

	replot_();
}

void ScatterPlot::replot_()
{
	//set axis ranges
	QBitArray filter;
	if (!params_.getBool("filtered")) filter = filter_;
	QPair<double, double> x_stats = col1_->getMinMax(filter);
	QPair<double, double> y_stats = col2_->getMinMax(filter);
	plot_->setAxisScale(QwtPlot::xBottom, x_stats.first, x_stats.second);
	plot_->setAxisScale(QwtPlot::yLeft, y_stats.first, y_stats.second);

	//format data (not filtered-out)
	double width = params_.getInt("line width");
	QPen pen(params_.getColor("color"), width);
	int size = params_.getInt("symbol size");
	curve_->setSymbol(new QwtSymbol(params_.getSymbol("symbol"), Qt::NoBrush, pen, QSize(size,size)));

	//format filterd-out data
	if (params_.getBool("filtered"))
	{
		QPen pen2(params_.getColor("filtered color"), width);
		curve2_->setSymbol(new QwtSymbol(params_.getSymbol("symbol"), Qt::NoBrush, pen2, QSize(size,size)));
		curve2_->setVisible(true);
	}
	else
	{
		curve2_->setVisible(false);
	}

	//format linear regression
	if (params_.getBool("linear regression"))
	{
		regression_->setVisible(true);
		regression_marker_->setVisible(true);
	}
	else
	{
		regression_->setVisible(false);
		regression_marker_->setVisible(false);
	}

	plot_->replot();
}
