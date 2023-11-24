#include <limits>
#include <QLineSeries>
#include <QDebug>
#include <QValueAxis>
#include "DataPlot.h"
#include "BasicStatistics.h"
#include "Exceptions.h"

DataPlot::DataPlot(QWidget *parent)
	: BasePlot(parent)
{
	//connect parameters, editor and plot
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(parameterChanged(QString)));

	//format plot
	chart_ = new QChart();
	chart_->legend()->setVisible(true);
	chart_->legend()->setAlignment(Qt::AlignRight);
	chart_->setBackgroundRoundness(0);
	chart_->setMargins(QMargins(0,0,0,0));

	//enable mouse tracking
	enableMouseTracking();
}

void DataPlot::setData(const DataSet& data, QList<int> cols, QString filename)
{
	//init
	filename_ = filename;
	QStringList line_types = QStringList() << "none" << "solid" << "dotted" << "dashed";
	params_.blockSignals(true);
	params_.clear();

	//check if there are duplicates names of series - use indices then
	QStringList names;
	for (int i=0; i<cols.count(); ++i)
	{
		names << data.column(cols[i]).headerOrIndex(cols[i]);
	}
	if (names.removeDuplicates()>0)
	{
		names.clear();
		for (int i=0; i<cols.count(); ++i)
		{
			names.append(data.column(cols[i]).headerOrIndex(cols[i], true));
		}
	}

	//create series
	QBitArray filter = data.getRowFilter(false);
	for (int i=0; i<cols.count(); ++i)
	{
		QString name = names[i];
		param_name_to_index_[name] = i;

		QLineSeries* series = new QLineSeries();
		series->setName(name);
		const NumericColumn& col = data.numericColumn(cols[i]);
		double pos = 1.0;
		for(int i=0; i<col.count(); ++i)
		{
			if (!filter.at(i)) continue;
			series->append(pos, col.value(i));
			pos += 1.0;
		}

		//add series (this sets the pen color)
		chart_->addSeries(series);

		//modify pen width
		QPen pen = series->pen();
		pen.setWidth(1);
		series->setPen(pen);

		//set parameters after adding the series
		params_.addColor(name + ":color", "", series->pen().color());
		params_.addInt(name + ":line width", "", series->pen().width(), 1, 999);
		params_.addString(name + ":line type", "", "solid", line_types);
		params_.addBool(name + ":points visible", "", false);
	}

	//format axes
	chart_->createDefaultAxes();
	QValueAxis* x_axis = qobject_cast<QValueAxis*>(chart_->axes(Qt::Horizontal).at(0));
	x_axis->setTitleText("data point");
	x_axis->setLabelFormat("%i");
	QValueAxis* y_axis = qobject_cast<QValueAxis*>(chart_->axes(Qt::Vertical).at(0));
	y_axis->setTitleText("value");

	//set parameters
	editor_->setParameters(params_);
	params_.blockSignals(false);

	//show chart
	chart_view_->setChart(chart_);

	//make legend markers clickable
	foreach(QLegendMarker* marker, chart_->legend()->markers())
	{
		connect(marker, SIGNAL(clicked()), this, SLOT(toggleSeriesVisibility()));
	}
}

QPen DataPlot::pen(QString series_name)
{
	//line
	QColor color = params_.getColor(series_name + ":color");
	int line_width = params_.getInt(series_name + ":line width");
	Qt::PenStyle style = Qt::SolidLine;
	QString line_type = params_.getString(series_name + ":line type");
	if (line_type=="none")
	{
		style = Qt::NoPen;
	}
	if (line_type=="dashed")
	{
		style = Qt::DashLine;
	}
	else if (line_type=="dotted")
	{
		style = Qt::DotLine;
	}
	return QPen(color, line_width, style);
}

bool DataPlot::pointsVisible(QString series_name)
{
	return params_.getBool(series_name + ":points visible");
}

void DataPlot::parameterChanged(QString series_and_parameter)
{
	int sep_idx = series_and_parameter.indexOf(':');
	if (sep_idx==-1) THROW(ProgrammingException, "Invalid parameter change format: " + series_and_parameter);
	QString series_name = series_and_parameter.left(sep_idx);
	int series_idx = param_name_to_index_[series_name];
	QString parameter = series_and_parameter.mid(sep_idx+1);

	QLineSeries* series = qobject_cast<QLineSeries*>(chart_->series().at(series_idx));
	if (parameter=="points visible")
	{
		series->setPointsVisible(pointsVisible(series_name));
	}
	else
	{
		series->setPen(pen(series_name));
	}
}
