#include <QBoxLayout>
#include <QPainter>
#include <QMenu>
#include <QToolButton>

#include "HistogramPlot.h"
#include "Histogram.h"
#include "ParameterEditor.h"

HistogramPlot::HistogramPlot(QWidget *parent)
	: BasePlot(parent)
	, data_(0)
	, column_(-1)
{
	//set default parameters
	params_.addInt("bins", "", 40, 1, 999);
	params_.addColor("color", "", Qt::darkBlue);
	params_.addDouble("min", "", 0.0);
	params_.addDouble("max", "", 0.0);
	params_.addSeparator();
	params_.addBool("percentage", "Show bin height as percentage value.", false);
	params_.addSeparator();
	params_.addBool("filtered", "Show filtered-out data points.", false);
	params_.addColor("filtered color", "", QColor(200, 0, 0));

	//connect parameters, editor and plot
	editor_->setParameters(params_);
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(replot_()));

	//attach plot
	attach(plot_);

	//enable mouse tracking
	enableMouseTracking(true);
}

void HistogramPlot::setData(DataSet& data, int column, QString filename)
{
	filename_ = filename;
	data_ = &data;
	column_ = column;

	replot_();
}

void HistogramPlot::replot_()
{
	bool show_filtered = params_.getBool("filtered");
	double bins = (double)params_.getInt("bins");
	bool percentage = params_.getBool("percentage");

	//format x-axis
	double min = params_.getDouble("min");
	double max = params_.getDouble("max");
	QBitArray filter;
	if (!show_filtered) filter = data_->getRowFilter(false);
	QPair<double, double> stats = data_->numericColumn(column_).getMinMax(filter);
	if (min>=max || min>=stats.second || max<=stats.first)
	{
		min = stats.first;
		max = stats.second;
	}
	plot()->setAxisScale(QwtPlot::xBottom, min, max);
	plot_->setAxisTitle(QwtPlot::xBottom, data_->column(column_).headerOrIndex(column_));

	//create histogram
	if (show_filtered) filter = data_->getRowFilter(false);
	hist_ = Histogram();
	hist_.init(min, max, (max-min)/bins, percentage);
	hist_.inc(data_->numericColumn(column_).values(filter), true);

	//create filtered histogram
	hist2_ = Histogram();
	if (show_filtered)
	{
		hist2_.init(min, max, (max-min)/bins, percentage);
		hist2_.inc(data_->numericColumn(column_).values(~filter), true);
	}

	//format y-axis
	if (percentage)
	{
		plot_->setAxisTitle(QwtPlot::yLeft, "Percentage");
	}
	else
	{
		plot_->setAxisTitle(QwtPlot::yLeft, "Counts");
	}
	double maxBin = hist_.maxValue();
	if (show_filtered)
	{
		maxBin = std::max(maxBin, hist2_.maxValue());
	}
	plot()->setAxisScale(QwtPlot::yLeft, -0.01*maxBin, 1.01*maxBin);

	//replot
	plot()->replot();
}

int HistogramPlot::rtti() const
{
	return QwtPlotItem::Rtti_PlotUserItem;
}

void HistogramPlot::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect) const
{
	bool show_filtered = params_.getBool("filtered");
	double half_bin_size = 0.4 * hist_.binSize();

	//filtered hist (below)
	if (show_filtered)
	{
		QColor color = params_.getColor("filtered color");
		color.setAlpha(180);
		for (int i = 0; i<hist2_.binCount(); ++i)
		{
			//do not paint a bar if there is no element in the bin
			double value = hist2_.binValue(i);
			if (value==0.0) continue;

			QRectF r = rect;
			r.setTop(yMap.transform(value) + 1);
			r.setBottom(yMap.transform(0.0) + 1);
			double center = hist2_.centerOfBin(i);
			r.setLeft(xMap.transform(center - half_bin_size) + 1);
			r.setRight(xMap.transform(center + half_bin_size) + 1);
			painter->fillRect(r, color);
		}
	}

	//main hist
	QColor color = params_.getColor("color");
	color.setAlpha(180);
	for (int i = 0; i<hist_.binCount(); ++i)
	{
		//do not paint a bar if there is no element in the bin
		double value = hist_.binValue(i);
		if (value==0.0) continue;

		QRectF r = rect;
		r.setTop(yMap.transform(value));
		r.setBottom(yMap.transform(0.0));
		double center = hist_.centerOfBin(i);
		r.setLeft(xMap.transform(center - half_bin_size));
		r.setRight(xMap.transform(center + half_bin_size));
		painter->fillRect(r, color);
	}
}
