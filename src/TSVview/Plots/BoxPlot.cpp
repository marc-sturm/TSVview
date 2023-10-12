#include <QBoxLayout>
#include <QPainter>
#include <QMenu>
#include <QToolButton>
#include <QBoxSet>
#include <QBoxPlotSeries>
#include <QDebug>
#include "BoxPlot.h"
#include "ParameterEditor.h"
#include "DataSet.h"

BoxPlot::BoxPlot(QWidget *parent)
	: BasePlot(parent)
{
	//set default parameters
	params_.addColor("color", "", Qt::darkBlue);
	params_.addSeparator();
	params_.addBool("filtered", "Show filtered-out data points.", false);
	params_.addColor("filtered color", "", QColor(200, 0, 0));

	//format plot
	chart_ = new QChart();
	chart_->legend()->setAlignment(Qt::AlignBottom);
	chart_->setBackgroundRoundness(0);
	chart_->setMargins(QMargins(0,0,0,0));

	//connect parameters, editor and plot
	editor_->setParameters(params_);
	connect(&params_, SIGNAL(valueChanged(QString)), this, SLOT(plot()));
}

void BoxPlot::setData(DataSet& data, QList<int> cols, QString filename)
{
	filename_ = filename;
	data_ = &data;
	cols_ = cols;

	plot();

	chart_view_->setChart(chart_);
}

void BoxPlot::plot()
{
	//clear
	chart_->removeAllSeries();

	//init
	QBitArray filter = data_->getRowFilter(false);

	//visible data series
	QBoxPlotSeries* series = new QBoxPlotSeries();
	series->setName("visible");
	series->setPen(QPen(params_.getColor("color")));
	series->setBrush(QBrush(params_.getColor("color", 170)));

	foreach(int c, cols_)
	{
		QBoxSet* set = new QBoxSet();

		QVector<double> values = data_->numericColumn(c).values(filter);
		if (values.count()==0)
		{
			values << BasicStatistics::mean(data_->numericColumn(c).values());
			set->setPen(QPen(Qt::transparent));
			set->setBrush(QBrush(Qt::transparent));
		}

		std::sort(values.begin(), values.end());

		set->setValue(QBoxSet::LowerExtreme, values.first());
		set->setValue(QBoxSet::LowerQuartile, BasicStatistics::q1(values, false));
		set->setValue(QBoxSet::Median, BasicStatistics::median(values, false));
		set->setValue(QBoxSet::UpperQuartile, BasicStatistics::q3(values, false));
		set->setValue(QBoxSet::UpperExtreme, values.last());
		set->setLabel(data_->column(c).headerOrIndex(c));

		series->append(set);
	}

	chart_->addSeries(series);

	//filtered data series
	bool show_filtered = params_.getBool("filtered");
	if (show_filtered)
	{
		QBoxPlotSeries* series2 = new QBoxPlotSeries();
		series2->setPen(QPen(params_.getColor("filtered color")));
		series2->setBrush(QBrush(params_.getColor("filtered color", 170)));
		series2->setName("filtered");

		foreach(int c, cols_)
		{
			QBoxSet* set = new QBoxSet();

			QVector<double> values = data_->numericColumn(c).values(~filter);
			if (values.count()==0)
			{
				values << BasicStatistics::mean(data_->numericColumn(c).values());
				set->setPen(QPen(Qt::transparent));
				set->setBrush(QBrush(Qt::transparent));
			}

			std::sort(values.begin(), values.end());

			set->setValue(QBoxSet::LowerExtreme, values.first());
			set->setValue(QBoxSet::LowerQuartile, BasicStatistics::q1(values, false));
			set->setValue(QBoxSet::Median, BasicStatistics::median(values, false));
			set->setValue(QBoxSet::UpperQuartile, BasicStatistics::q3(values, false));
			set->setValue(QBoxSet::UpperExtreme, values.last());
			set->setLabel(data_->column(c).headerOrIndex(c));

			series2->append(set);
		}

		chart_->addSeries(series2);
	}

	//axes
	chart_->createDefaultAxes();

	//legend
	chart_->legend()->setVisible(show_filtered);
}
