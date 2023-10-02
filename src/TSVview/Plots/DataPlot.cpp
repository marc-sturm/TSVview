#include <limits>
#include <QLineSeries>
#include <QDebug>
#include "DataPlot.h"
#include "BasicStatistics.h"

DataPlot::DataPlot(QWidget *parent)
	: BasePlot(parent)
{
	//connect parameters, editor and plot
	//TODO connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(replot_()));

	//format plot
	chart_ = new QChart();
	chart_->legend()->setVisible(true);
	chart_->legend()->setAlignment(Qt::AlignRight);

	//plots needs to be redrawn if legend items are checked/unckeched
	//TODO connect(legend, SIGNAL(checked(QVariant,bool,int)), this, SLOT(legendChecked_(QVariant,bool,int)));

	//enable mouse tracking
	enableMouseTracking(true);
}

void DataPlot::setData(DataSet& data, QList<int> cols, QString filename)
{
	filename_ = filename;

	//check if curve names are equal (use index then)
	QVector<QString> names;
	for (int i=0; i<cols.count(); ++i)
	{
		names.append(data.column(cols[i]).headerOrIndex(cols[i]));
	}

	bool name_collision = false;
	for (int i=0; i<names.count(); ++i)
	{
		for (int j=i+1; j<names.count(); ++j)
		{
			if (names[i]==names[j])
			{
				name_collision = true;
			}
		}
	}
	if (name_collision)
	{
		names.clear();
		for (int i=0; i<cols.count(); ++i)
		{
			names.append(data.column(cols[i]).headerOrIndex(cols[i], true));
		}
	}

	//create curves
	QBitArray filter = data.getRowFilter(false);
	for (int i=0; i<cols.count(); ++i)
	{
		QLineSeries* series = new QLineSeries();
		series->setName(names[i]);

		const NumericColumn& col = data.numericColumn(cols[i]);
		double pos = 1.0;
		for(int i=0; i<col.count(); ++i)
		{
			if (!filter.at(i)) continue;
			series->append(pos, col.value(i));
			pos += 1.0;
		}

		chart_->addSeries(series);
	}
	chart_->createDefaultAxes();
	chart_view_->setChart(chart_);
/*
	//create parameters
	params_.blockSignals(true);
	params_.clear();
	QStringList line_types;
	line_types << "none" << "solid" << "dotted" << "dashed";
	for (int i=0; i<series_.count(); ++i)
	{
		QString section = series_.at(i)->title().text();
		params_.addColor(section + ":color", "", getColor_(i));
		params_.addInt(section + ":line width", "", 1, 1, 999);
		params_.addString(section + ":line type", "", getLineType_(i), line_types);
		params_.addSymbol(section + ":symbol", "", QwtSymbol::NoSymbol);
		params_.addInt(section + ":symbol size", "", 6, 2, 999);
		params_.addInt(section + ":symbol line width", "", 1, 1, 999);
	}
	editor_->setParameters(params_);
	params_.blockSignals(false);

	//dynamic formatting of plot
	double y_min = std::numeric_limits<double>::max();
	double y_max = -std::numeric_limits<double>::max();
	for (int i=0; i<cols.count(); ++i)
	{
		QPair<double, double> stats = data.numericColumn(cols[i]).getMinMax(filter);
		y_min = std::min(y_min, stats.first);
		y_max = std::max(y_max, stats.second);
	}
	plot_->setAxisScale(QwtPlot::yLeft, y_min, y_max);
	plot_->setAxisTitle(QwtPlot::yLeft, QwtText("value"));
	plot_->setAxisScale(QwtPlot::xBottom, 1, size);
	plot_->setAxisTitle(QwtPlot::xBottom, QwtText("data point"));

	replot_();

	//add panner
	QwtPlotPanner* panner = new QwtPlotPanner(plot_->canvas());
	panner->setMouseButton(Qt::LeftButton, Qt::ControlModifier);

	//add zoomer
	QwtPlotZoomer* zoomer_ = new QwtPlotZoomer(plot_->canvas(), true);
	zoomer_->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
	zoomer_->setTrackerMode(QwtPicker::AlwaysOff);
	zoomer_->setZoomBase(QRectF(0, y_min, size, y_max-y_min));
*/
}

void DataPlot::replot_()
{
	/*
	for (int i=0; i<series_.count(); ++i)
	{
		//set line pen
		QString section = series_.at(i)->title().text();
		QColor color = params_.getColor(section + ":color");
		QString line_type = params_.getString(section + ":line type");
		if (line_type!="none")
		{
			int line_width = params_.getInt(section + ":line width");
			if (line_type=="solid")
			{
				series_[i]->setPen(QPen(color, line_width, Qt::SolidLine));
			}
			else if (line_type=="dashed")
			{
				series_[i]->setPen(QPen(color, line_width, Qt::DashLine));
			}
			else if (line_type=="dotted")
			{
				series_[i]->setPen(QPen(color, line_width, Qt::DotLine));
			}
		}
		else
		{
			series_[i]->setPen(QPen(Qt::NoPen));
		}

		//set symbol
		QwtSymbol::Style symbol_type = params_.getSymbol(section + ":symbol");
		if (symbol_type!=QwtSymbol::NoSymbol)
		{
			int size = params_.getInt(section + ":symbol size");
			int line_width = params_.getInt(section + ":symbol line width");
			series_[i]->setSymbol(new QwtSymbol(symbol_type, Qt::NoBrush, QPen(color, line_width), QSize(size,size)));
		}
		else
		{
			series_[i]->setSymbol(new QwtSymbol());
		}
	}

	//replot
	plot_->replot();
	*/
}

void DataPlot::legendChecked_(const QVariant& info, bool on, int /*index*/)
{
//TODO	plot_->infoToItem(info)->setVisible(!on);
//TODO	plot_->replot();
}

QColor DataPlot::getColor_(int i)
{
	i = i%6;

	if (i==0)
	{
		return QColor(0, 0, 220);
	}
	else if (i==1)
	{
		return QColor(220, 0, 0);
	}
	else if (i==2)
	{
		return QColor(0, 170, 0);
	}
	else if (i==3)
	{
		return QColor(170, 170, 0);
	}
	else if (i==4)
	{
		return QColor(170, 0, 170);
	}
	else
	{
		return QColor(0, 0, 0);
	}
}

QString DataPlot::getLineType_(int i)
{
	i = i%12;

	if (i>=6)
	{
		return "dashed";
	}
	else
	{
		return "solid";
	}
}
