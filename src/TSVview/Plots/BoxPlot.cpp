#include <QBoxLayout>
#include <QPainter>
#include <QMenu>
#include <QToolButton>
#include <math.h>

#include "BoxPlot.h"
#include "ParameterEditor.h"
#include "DataSet.h"

BoxPlot::BoxPlot(QWidget *parent)
	: BasePlot(parent)
	, data_(0)
	, cols_()
{
	//set default parameters
	params_.addColor("color", "", Qt::darkBlue);
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

void BoxPlot::setData(DataSet& data, QList<int> cols, QString filename)
{
	filename_ = filename;
	data_ = &data;
	cols_ = cols;

	replot_();
}

void BoxPlot::replot_()
{
	bool show_filtered = params_.getBool("filtered");

	QBitArray filter;
	if (!show_filtered) filter = data_->getRowFilter(false);

	//dynamic formatting of plot
	double min = std::numeric_limits<double>::max();
	double max = -std::numeric_limits<double>::max();
	for (int i=0; i<cols_.count(); ++i)
	{
		QPair<double, double> stats = data_->numericColumn(cols_[i]).getMinMax(filter);
		min = std::min(min, stats.first);
		max = std::max(max, stats.second);
	}
	double delta = fabs(max-min);
	max += 0.07*delta;

	plot_->setAxisScale(QwtPlot::yLeft,  min, max);
	plot_->setAxisScale(QwtPlot::xBottom,-0.6+1.0, cols_.count()-0.4+1.0, 100);
	plot_->setAxisMaxMinor(QwtPlot::xBottom, 0);

	//replot
	plot()->replot();
}

int BoxPlot::rtti() const
{
	return QwtPlotItem::Rtti_PlotUserItem;
}

void BoxPlot::draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF & /*rect*/) const
{
	bool show_filtered = params_.getBool("filtered");
	QColor color = params_.getColor("color");
	QColor color2 = params_.getColor("filtered color");

	for (int i = 0; i<cols_.count(); ++i)
	{
		//draw label
		QString label = data_->column(cols_[i]).headerOrIndex(cols_[i]);
		QwtText text(label);
		painter->setPen(QPen(Qt::black));
		int left = xMap.transform(i-0.4+1.0);
		int right = xMap.transform(i+0.4+1.0);
		text.draw(painter, QRect(left, 3, right-left, 20));

		//draw filterd box (below)
		QBitArray filter = data_->getRowFilter(false);
		if (show_filtered)
		{
			drawBox(painter, xMap, yMap, i, color2, ~filter, 2);
		}

		//draw box
		drawBox(painter, xMap, yMap, i, color, filter);
	}
}

void BoxPlot::drawBox(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, int col, QColor color, QBitArray filter, int offset) const
{
	QPen pen1 = QPen(color, 1);
	QPen pen2 = QPen(color, 2);

	// Calculate values to draw
	QVector<double> data = data_->numericColumn(cols_[col]).values(filter);
	StatisticsSummary stats = data_->numericColumn(cols_[col]).statistics(filter);

	int left = xMap.transform(col-0.4+1.0) + offset;
	int middle = xMap.transform(col+1.0) + offset;
	int right = xMap.transform(col+0.4+1.0) + offset;

	double threshold = std::min(stats.max, stats.q3 + 1.5*(stats.q3-stats.q1));
	double max_value = stats.median;
	for (int j=0; j<data.count(); ++j)
	{
		double value = data[j];
		if (value<=threshold && value>max_value)
		{
			max_value = value;
		}
	}
	int max = yMap.transform(max_value);

	int q3 = yMap.transform(stats.q3);
	int med = yMap.transform(stats.median);
	int q1 = yMap.transform(stats.q1);

	threshold = std::max(stats.min, stats.q1 - 1.5*(stats.q3-stats.q1));
	double min_value = stats.median;
	for (int j=0; j<data.count(); ++j)
	{
		double value = data[j];
		if (value>=threshold && value<min_value)
		{
			min_value = value;
		}
	}
	int min = yMap.transform(min_value);

	//draw boxe and sticks
	painter->setPen(pen1);

	//box (q1, q3)
	painter->drawRect(left, q1, right-left, q3-q1);

	//median line
	painter->drawLine(left, med, right, med);

	//min line
	painter->drawLine(left, min, right, min);
	painter->drawLine(middle, min, middle, q1);

	//max line
	painter->drawLine(left, max, right, max);
	painter->drawLine(middle, max, middle, q3);

	//draw outliers
	painter->setPen(pen2);
	for (int j=0; j<data.count(); ++j)
	{
		double value = data[j];
		if (value<min_value)
		{
			int pos = yMap.transform(value);
			painter->drawPoint(middle, pos);
		}
		else if (value>max_value)
		{
			int pos = yMap.transform(value);
			painter->drawPoint(middle, pos);
		}
	}
}
