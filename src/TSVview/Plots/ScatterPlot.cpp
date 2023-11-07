#include <QDebug>
#include <QLineSeries>
#include "Helper.h"
#include "ScatterPlot.h"
#include "StatisticsSummary.h"
#include "BasicStatistics.h"
#include "math.h"

ScatterPlot::ScatterPlot(QWidget *parent)
	: BasePlot(parent)
{
	//set default parameters
	params_.addColor("color", "", Qt::darkBlue);
	params_.addInt("symbol size", "", 5, 1, 999);
	params_.addSeparator();
	params_.addBool("linear regression", "Show linear regression of unfiltered data.", false);
	params_.addSeparator();
	params_.addBool("filtered", "Show filtered-out data points.", false);
	params_.addColor("filtered color", "", QColor(200, 0, 0));
	params_.addInt("filtered symbol size", "", 5, 1, 999);
	params_.addSeparator();
	params_.addInt("position noise", "", 0 , 0, 20);

	//connect parameters, editor and plot
	editor_->setParameters(params_);
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(parameterChanged(QString)));

	//format plot
	chart_ = new QChart();
	chart_->legend()->setVisible(false);
	chart_->setBackgroundRoundness(0);
	chart_->setMargins(QMargins(0,0,0,0));
	chart_->setDropShadowEnabled(false);

	//enable mouse tracking
	enableMouseTracking();
}

void ScatterPlot::setData(const DataSet& data, int col1, int col2, QString filename)
{
	filter_ = data.getRowFilter(false);
	col1_ = data.numericColumn(col1).values();
	col2_ = data.numericColumn(col2).values();
	filename_ = filename;

	//create series of visible data
	addSeries();

	//set axes labels
	chart_->axes(Qt::Horizontal).at(0)->setTitleText(data.numericColumn(col1).headerOrIndex(col1));
	chart_->axes(Qt::Vertical).at(0)->setTitleText(data.numericColumn(col2).headerOrIndex(col2));

	//show chart
	chart_view_->setChart(chart_);
}

void ScatterPlot::parameterChanged(QString parameter)
{
	if (parameter=="color")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("visible"));
		setSymbol(series, params_.getInt("symbol size"), params_.getColor("color"));
	}
	else if (parameter=="symbol size")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("visible"));
		setSymbol(series, params_.getInt("symbol size"), params_.getColor("color"));
	}
	else if (parameter=="linear regression")
	{
		QLineSeries* series = qobject_cast<QLineSeries*>(search("regression"));
		if (series!=nullptr)
		{
			chart_->removeSeries(series);
			info_label_->clear();
		}
		else
		{
			//calculate linear regression
			QVector<double> x;
			QVector<double> y;
			for (int i=0; i<filter_.count(); ++i)
			{
				if (filter_[i])
				{
					x << col1_[i];
					y << col2_[i];
				}
			}

			QPair<double, double> reg = BasicStatistics::linearRegression(x, y);
			double offset = reg.first;
			double slope = reg.second;

			//create linear regression plot series
			series = new QLineSeries();
			series->setName("regression");
			auto x_min_max = BasicStatistics::getMinMax(x);
			series->append(x_min_max.first, offset + slope * x_min_max.first);
			series->append(x_min_max.second, offset + slope * x_min_max.second);
			series->setColor(Qt::darkGray);
			chart_->addSeries(series);
			series->attachAxis(chart_->axes(Qt::Horizontal).at(0));
			series->attachAxis(chart_->axes(Qt::Vertical).at(0));

			//calcualte R-squared
			double y_mean = BasicStatistics::mean(y);
			double data_diff = 0.0;
			double model_diff = 0.0;
			for (int i=0; i<y.size(); ++i)
			{
				if (BasicStatistics::isValidFloat(x[i]) && BasicStatistics::isValidFloat(y[i]))
				{
					model_diff += pow(offset + slope * x[i] - y_mean, 2.0);
					data_diff += pow(y[i] - y_mean, 2.0);
				}
			}
			double r_squared = model_diff / data_diff;
			info_label_->setText("R²=" + QString::number(r_squared, 'f', 5));
		}
	}
	else if (parameter=="filtered")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("filtered"));
		if (series!=nullptr)
		{
			chart_->removeSeries(series);
		}
		else
		{
			addSeriesFiltered();
		}

		//reset zoom range
		QRectF bb = getBoundingBox();
		chart_->axes(Qt::Horizontal).at(0)->setRange(bb.x(), bb.x() + bb.width());
		chart_->axes(Qt::Vertical).at(0)->setRange(bb.y() + bb.height(), bb.y());
	}
	else if (parameter=="filtered color")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("filtered"));
		setSymbol(series, params_.getInt("filtered symbol size"), params_.getColor("filtered color"));
	}
	else if (parameter=="filtered symbol size")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("filtered"));
		setSymbol(series, params_.getInt("filtered symbol size"), params_.getColor("filtered color"));
	}
	else if (parameter=="position noise")
	{
		//visible
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("visible"));
		chart_->removeSeries(series);
		addSeries();

		//filtered
		series = qobject_cast<QScatterSeries*>(search("filtered"));
		if (series!=nullptr)
		{
			chart_->removeSeries(series);
			addSeriesFiltered();
		}
	}
}

void ScatterPlot::addSeries()
{
	double noise_perc_x = params_.getInt("position noise") / 100.0;
	double noise_perc_y = params_.getInt("position noise") / 100.0;
	bool add_noise = noise_perc_x>0;
	if (add_noise)
	{
		QRectF bb = getBoundingBox();
		noise_perc_x *= bb.width();
		noise_perc_y *= bb.height();
	}

	QScatterSeries* series = new QScatterSeries();
	series->setName("visible");
	setSymbol(series, params_.getInt("symbol size"), params_.getColor("color"));
	for(int i=0; i<filter_.count(); ++i)
	{
		if (filter_[i])
		{
			double x = col1_.value(i);
			double y =  col2_.value(i);
			if (add_noise)
			{
				x += Helper::randomNumber(-1,1) * noise_perc_x;
				y += Helper::randomNumber(-1,1) * noise_perc_y;
			}
			series->append(x, y);
		}
	}
	chart_->addSeries(series);

	//add/attach axes
	if (chart_->axes().count()==0)
	{
		chart_->createDefaultAxes();
	}
	else
	{
		series->attachAxis(chart_->axes(Qt::Horizontal).at(0));
		series->attachAxis(chart_->axes(Qt::Vertical).at(0));
	}
}

void ScatterPlot::addSeriesFiltered()
{
	double noise_perc_x = params_.getInt("position noise") / 100.0;
	double noise_perc_y = params_.getInt("position noise") / 100.0;
	bool add_noise = noise_perc_x>0;
	if (add_noise)
	{
		QRectF bb = getBoundingBox();
		noise_perc_x *= bb.width();
		noise_perc_y *= bb.height();
	}

	QScatterSeries* series = new QScatterSeries();
	series->setName("filtered");
	setSymbol(series, params_.getInt("filtered symbol size"), params_.getColor("filtered color"));
	for(int i=0; i<filter_.count(); ++i)
	{
		if (!filter_[i])
		{
			double x = col1_.value(i);
			double y = col2_.value(i);
			if (add_noise)
			{
				x += Helper::randomNumber(-1,1) * noise_perc_x;
				y += Helper::randomNumber(-1,1) * noise_perc_y;
			}
			series->append(x, y);
		}
	}
	chart_->addSeries(series);
	series->attachAxis(chart_->axes(Qt::Horizontal).at(0));
	series->attachAxis(chart_->axes(Qt::Vertical).at(0));
}

QRectF ScatterPlot::getBoundingBox() const
{
	bool use_filtered = params_.getBool("filtered");
	double x_min = std::numeric_limits<qreal>::max();
	double x_max = -std::numeric_limits<qreal>::max();
	double y_min = x_min;
	double y_max = x_max;

	for (int i=0; i<filter_.count(); ++i)
	{
		if (use_filtered || filter_[i])
		{
			double x = col1_[i];
			double y = col2_[i];
			if (x<x_min) x_min = x;
			if (x>x_max) x_max = x;
			if (y<y_min) y_min = y;
			if (y>y_max) y_max = y;
		}
	}

	return QRectF(QPointF(x_min, y_max), QPointF(x_max, y_min));
}

void ScatterPlot::setSymbol(QScatterSeries* series, int size, QColor color)
{
	size+=2; //we need a transparent 1px border - otherwise the strage artefacts can occur at the borders

	series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
	series->setMarkerSize(size);

	QImage cross(size, size, QImage::Format_ARGB32);
	cross.fill(Qt::transparent);

	QPainter painter(&cross);
	painter.setPen(color);
	painter.setBrush(Qt::transparent);
	painter.drawLine(1,1,size-2,size-2);
	painter.drawLine(1,size-2,size-2,1);

	series->setBrush(cross);
	series->setPen(QPen(Qt::transparent));
}
