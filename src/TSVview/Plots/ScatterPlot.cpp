#include <QDebug>

#include "Helper.h"
#include "ScatterPlot.h"
#include "StatisticsSummary.h"
#include "BasicStatistics.h"

ScatterPlot::ScatterPlot(QWidget *parent)
	: BasePlot(parent)
	, curve_(0)
	, curve_noisy_(0)
	, curve2_(0)
	, curve2_noisy_(0)
	, regression_(0)
	, regression_marker_(0)
{
	//set default parameters
	params_.addColor("color", "", Qt::darkBlue);
	params_.addInt("line width", "", 2, 1, 999);
	params_.addSymbol("symbol", "", QwtSymbol::XCross);
	params_.addInt("symbol size", "", 5, 2, 999);
	params_.addSeparator();
	params_.addInt("position noise", "", 0 , 0, 20);
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

QVector<double> ScatterPlot::addNoise(QVector<double> data, double noise)
{
	for (int i=0; i<data.count(); ++i)
	{
		data[i] += Helper::randomNumber(-1,1) * noise;
	}
	return data;
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
	x_ = col1_->values(filter_);
	y_ = col2_->values(filter_);
	curve_ = addCurve();
	curve_->setSamples(x_, y_);
	curve_noisy_ = addCurve();
	curve_noisy_->setSamples(x_, y_);

	//create curve (filtered-out data)
	x2_ = col1_->values(~filter_);
	y2_ = col2_->values(~filter_);
	curve2_ = addCurve();
	curve2_->setSamples(x2_, y2_);
	curve2_noisy_ = addCurve();
	curve2_noisy_->setSamples(x_, y_);

	//calculate linear regression
	QPair<double, double> reg = BasicStatistics::linearRegression(x_, y_);
	double offset = reg.first;
	double slope = reg.second;

	// calcualte R-squared
	StatisticsSummary x_stats = col1_->statistics(filter_);
	StatisticsSummary y_stats = col2_->statistics(filter_);
	double mean = y_stats.mean;
	double data_diff = 0.0;
	double model_diff = 0.0;
	for (int i=0; i<y_.size(); ++i)
	{
		if (BasicStatistics::isValidFloat(x_[i]) && BasicStatistics::isValidFloat(y_[i]))
		{
			model_diff += pow(offset + slope * x_[i] - mean, 2.0);
			data_diff += pow(y_[i] - mean, 2.0);
		}
	}
	double r_squared = model_diff / data_diff;

	//create linear regression plot curve
	regression_ = new QwtPlotCurve();
	QVector<double> x;
	x.append(x_stats.min);
	x.append(x_stats.max);
	QVector<double> y;
	y.append(offset + slope * x_stats.min);
	y.append(offset + slope * x_stats.max);
	regression_->setSamples(x, y);
	regression_->attach(plot_);

	// create regression marker
	regression_marker_ = new QwtPlotMarker();
	regression_marker_->setXValue(x_stats.max);
	regression_marker_->setYValue(y_stats.min);
	regression_marker_->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
	regression_marker_->setLabel(QwtText("R² = " + QString::number(r_squared)));
	regression_marker_->attach(plot_);

	replot_();
}

QwtPlotCurve* ScatterPlot::addCurve()
{
	QwtPlotCurve* curve = new QwtPlotCurve();
	curve->setStyle(QwtPlotCurve::NoCurve);
	curve->setTitle("title");
	curve->attach(plot_);

	return curve;
}

void ScatterPlot::removeCurve(QwtPlotCurve* curve)
{
	if (curve!=0)
	{
		curve->detach();
		delete curve;
	}
}

void ScatterPlot::formatCurve(QwtPlotCurve* curve, QString color_name, bool visible)
{
	if (curve==0) return;

	double line_width = params_.getInt("line width");
	QPen pen(params_.getColor(color_name), line_width);
	int symbol_size = params_.getInt("symbol size");
	curve->setSymbol(new QwtSymbol(params_.getSymbol("symbol"), Qt::NoBrush, pen, QSize(symbol_size, symbol_size)));
	curve->setVisible(visible);
}

void ScatterPlot::replot_()
{
	//get noise percentage
	double noise_perc = params_.getInt("position noise") / 100.0;

	//set axis ranges
	QBitArray filter;
	if (!params_.getBool("filtered")) filter = filter_;
	QPair<double, double> x_stats = col1_->getMinMax(filter);
	QPair<double, double> y_stats = col2_->getMinMax(filter);
	double x_range = x_stats.second - x_stats.first;
	double y_range = y_stats.second - y_stats.first;
	plot_->setAxisScale(QwtPlot::xBottom, x_stats.first - noise_perc * x_range, x_stats.second + noise_perc * x_range);
	plot_->setAxisScale(QwtPlot::yLeft, y_stats.first - noise_perc * y_range, y_stats.second + noise_perc * y_range);

	//update noisy curves if needed
	if (noise_perc>0)
	{
		removeCurve(curve_noisy_);
		curve_noisy_ = addCurve();
		curve_noisy_->setSamples(addNoise(x_, noise_perc * x_range), addNoise(y_, noise_perc * y_range));

		removeCurve(curve2_noisy_);
		curve2_noisy_ = addCurve();
		curve2_noisy_->setSamples(addNoise(x2_, noise_perc * x_range), addNoise(y2_, noise_perc * y_range));
	}

	//format curves (not filtered-out)
	formatCurve(curve_, "color", noise_perc==0);
	formatCurve(curve_noisy_, "color", noise_perc>0);

	//format curves (filterd-out)
	bool show_filtered = params_.getBool("filtered");
	formatCurve(curve2_, "filtered color", show_filtered && noise_perc==0);
	formatCurve(curve2_noisy_, "filtered color", show_filtered && noise_perc>0);

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
