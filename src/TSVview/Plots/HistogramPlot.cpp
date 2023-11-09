#include <QBoxLayout>
#include <QPainter>
#include <QMenu>
#include <QToolButton>
#include <QBarSet>
#include <QBarSeries>
#include <QDebug>
#include <QBarCategoryAxis>
#include "HistogramPlot.h"
#include "Histogram.h"
#include "ParameterEditor.h"

HistogramPlot::HistogramPlot(QWidget *parent)
	: BasePlot(parent)
{
	//set default parameters
	params_.addInt("bins", "", 30, 1, 999);
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
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(parameterChanged(QString)));

	//format plot
	chart_ = new QChart();
	chart_->setBackgroundRoundness(0);
	chart_->setMargins(QMargins(0,0,0,0));
	chart_->setDropShadowEnabled(false);
}

void HistogramPlot::setData(DataSet& data, int column, QString filename)
{
	filename_ = filename;
	filter_ = data.getRowFilter(false);
	col_ = data.numericColumn(column).values();
	name_ = data.column(column).headerOrIndex(column);

	plot();

	chart_view_->setChart(chart_);
}

void HistogramPlot::parameterChanged(QString /*parameter*/)
{
	plot();
}

void HistogramPlot::plot()
{
	//init
	bool show_filtered = params_.getBool("filtered");
	double bins = (double)params_.getInt("bins");
	bool percentage = params_.getBool("percentage");

	//remove previous series if exist
	chart_->removeAllSeries();

	//create new series
	QBarSeries* barseries = new QBarSeries();

	//determine min and max
	double min = params_.getDouble("min");
	double max = params_.getDouble("max");
	QPair<double, double> stats = getMinMax();
	if (min>=max || min>=stats.second || max<=stats.first)
	{
		min = stats.first;
		max = stats.second;
	}

	//bar set of visible data
	Histogram hist(min, max, (max-min)/bins);
	for (int i=0; i<filter_.count(); ++i)
	{
		if (filter_[i])
		{
			hist.inc(col_[i], true);
		}
	}
	QBarSet* set = new QBarSet("visible");
	set->setColor(params_.getColor("color"));
	for (int i=0; i<hist.binCount(); ++i)
	{
		set->append(hist.binValue(i, percentage));
	}
	barseries->append(set);

	//bar set of filtered data
	if (show_filtered)
	{
		Histogram hist2(min, max, (max-min)/bins);
		for (int i=0; i<filter_.count(); ++i)
		{
			if (!filter_[i])
			{
				hist2.inc(col_[i], true);
			}
		}
		QBarSet* set2 = new QBarSet("filtered");
		set2->setColor(params_.getColor("filtered color"));
		for (int i=0; i<hist2.binCount(); ++i)
		{
			set2->append(hist2.binValue(i, percentage));
		}
		barseries->append(set2);
	}

	//create chart
	chart_->addSeries(barseries);
	chart_->setTitle(name_);
	chart_->legend()->setVisible(show_filtered);
	chart_->legend()->setAlignment(Qt::AlignBottom);
	chart_->createDefaultAxes();

	//y axix
	chart_->axes(Qt::Vertical).at(0)->setTitleText(percentage ? "%" : "count");

	//x axix
	QStringList labels;
	QBarCategoryAxis* axis = new QBarCategoryAxis();
	double bin_size = hist.binSize();
	for (int i=0; i<hist.binCount(); ++i)
	{
		double start = hist.startOfBin(i);
		double end = start + bin_size;
		labels << QString::number(start) + " > " +  QString::number(end);
	}
	axis->append(labels);
	axis->setLabelsAngle(90);
	chart_->removeAxis(chart_->axes(Qt::Horizontal).at(0));
	chart_->addAxis(axis, Qt::AlignBottom);
	barseries->attachAxis(axis);
}

QPair<double, double> HistogramPlot::getMinMax()
{
	double min = std::numeric_limits<qreal>::max();
	double max = -std::numeric_limits<qreal>::max();

	bool show_filtered = params_.getBool("filtered");
	for (int i=0; i<filter_.count(); ++i)
	{
		if (show_filtered || filter_[i])
		{
			if (col_[i]<min) min = col_[i];
			if (col_[i]>max) max = col_[i];
		}
	}

	return qMakePair<double, double>(min, max);
}
