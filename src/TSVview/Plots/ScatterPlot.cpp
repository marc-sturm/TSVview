#include <QDebug>
#include <QLineSeries>
#include "Helper.h"
#include "ScatterPlot.h"
#include "BasicStatistics.h"
#include "Exceptions.h"
#include "math.h"
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QLegendMarker>
#include <QSignalBlocker>

ScatterPlot::ScatterPlot(QWidget *parent)
	: BasePlot(parent)
	, x_is_date_(false)
	, color_by_column_(false)
{
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

void ScatterPlot::setData(const DataSet& data, int col1, int col2, QString filename, int color_col)
{
	//Validate and assign colors before modifying the plot. Include all rows so
	//filtering and skipped coordinates do not affect the category mapping.
	if (color_col < -1 || color_col >= data.columnCount())
	{
		THROW(ArgumentException, "Invalid scatterplot color column index: " + QString::number(color_col));
	}
	QVector<QColor> row_colors;
	QStringList category_names;
	QVector<QColor> legend_colors;
	if (color_col != -1)
	{
		//Matplotlib's classic b, g, r, c, m, y, k cycle, followed by
		//additional colors to support 20 categories without repeating the cycle.
		static const QVector<QColor> palette = {
			QColor("#0000ff"), QColor("#008000"), QColor("#ff0000"), QColor("#00bfbf"),
			QColor("#bf00bf"), QColor("#bfbf00"), QColor("#000000"),
			QColor("#ff8000"), QColor("#8000ff"), QColor("#a52a2a"), QColor("#ff1493"),
			QColor("#008080"), QColor("#80c000"), QColor("#4169e1"), QColor("#800000"),
			QColor("#808080"), QColor("#d2691e"), QColor("#00a050"), QColor("#c060c0"),
			QColor("#c0a060")
		};
		QHash<QString, QColor> category_colors;
		row_colors.reserve(data.rowCount());
		for (int row=0; row<data.rowCount(); ++row)
		{
			const QString value = data.column(color_col).string(row);
			if (!category_colors.contains(value))
			{
				if (category_colors.size() == palette.size())
				{
					THROW(ArgumentException, "Scatterplot color column '" + data.column(color_col).headerOrIndex(color_col) + "' contains more than 20 distinct values.");
				}
				const QColor color = palette.at(category_colors.size());
				category_colors.insert(value, color);
				category_names.append(value);
				legend_colors.append(color);
			}
			row_colors.append(category_colors.value(value));
		}
	}
	color_by_column_ = color_col != -1;
	row_colors_ = row_colors;

	//set parameters for the selected coloring mode
	const QSignalBlocker blocker(&params_);
	params_.clear();
	if (!color_by_column_) params_.addColor("color", "", Qt::darkBlue);
	params_.addInt("symbol size", "", 5, 1, 999);
	params_.addSeparator();
	params_.addBool("linear regression", "Show linear regression of unfiltered data.", false);
	params_.addSeparator();
	params_.addBool("filtered", "Show filtered-out data points.", false);
	if (!color_by_column_) params_.addColor("filtered color", "", QColor(200, 0, 0));
	params_.addInt("filtered symbol size", "", color_by_column_ ? 3 : 5, 1, 999);
	params_.addSeparator();
	params_.addInt("position noise", "", 0 , 0, 20);

	//populate the editor once the coloring mode is known
	editor_->setParameters(params_);

	//one date and one numeric column > make sure date column is X
	BaseColumn::Type type1 = data.column(col1).type();
	BaseColumn::Type type2 = data.column(col2).type();
	if (type1==BaseColumn::NUMERIC && type2==BaseColumn::DATE)
	{
		int tmp = col1;
		col1 = col2;
		col2 = tmp;
		x_is_date_ = true;
	}
	else if (type1==BaseColumn::DATE && type2==BaseColumn::NUMERIC)
	{
		x_is_date_ = true;
	}

	//determine rows that are skipped because of non-numeric data
	filter_ = data.getRowFilter(false);
	if (x_is_date_)
	{
		values_x_.clear();
		foreach(const QDate& date, data.dateColumn(col1).values())
		{
			values_x_ << (date.isValid() ? date.startOfDay().toMSecsSinceEpoch() : std::numeric_limits<double>::quiet_NaN());
		}
	}
	else
	{
		values_x_ = data.numericColumn(col1).values();
	}
	values_y_ = data.numericColumn(col2).values();
	filename_ = filename;

	//create series of visible data
	addSeries();

	updateAxisRanges();

	//set axes labels
	chart_->axes(Qt::Horizontal).at(0)->setTitleText(data.column(col1).headerOrIndex(col1));
	chart_->axes(Qt::Vertical).at(0)->setTitleText(data.column(col2).headerOrIndex(col2));

	//Empty series provide category legend entries without adding plotted points.
	//Keep their internal names separate from the visible/filtered/regression names.
	chart_->legend()->setVisible(color_by_column_);
	if (color_by_column_)
	{
		chart_->legend()->setAlignment(Qt::AlignRight);
		for (int i=0; i<category_names.size(); ++i)
		{
			QScatterSeries* entry = new QScatterSeries();
			entry->setName("category legend " + QString::number(i));
			entry->setMarkerShape(QScatterSeries::MarkerShapeCircle);
			entry->setColor(legend_colors.at(i));
			entry->setBorderColor(legend_colors.at(i));
			chart_->addSeries(entry);
			for (QLegendMarker* marker : chart_->legend()->markers(entry))
			{
				marker->setShape(QLegend::MarkerShapeCircle);
				marker->setLabel(category_names.at(i).isEmpty() ? "(empty)" : category_names.at(i));
				marker->setBrush(legend_colors.at(i));
				marker->setPen(QPen(Qt::NoPen));
			}
		}
	}

	//show chart
	chart_view_->setChart(chart_);
}

void ScatterPlot::parameterChanged(QString parameter)
{
	if (parameter=="color")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("visible"));
		setSymbol(series, params_.getInt("symbol size"), (color_by_column_ ? QColor(Qt::darkBlue) : params_.getColor("color")));
	}
	else if (parameter=="symbol size")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("visible"));
		setSymbol(series, params_.getInt("symbol size"), (color_by_column_ ? QColor(Qt::darkBlue) : params_.getColor("color")));
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
					x << values_x_[i];
					y << values_y_[i];
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
			if (color_by_column_)
			{
				for (QLegendMarker* marker : chart_->legend()->markers(series)) marker->setVisible(false);
			}
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
			addSeries(true);
		}

		updateAxisRanges();
	}
	else if (parameter=="filtered color")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("filtered"));
		setSymbol(series, params_.getInt("filtered symbol size"), (color_by_column_ ? QColor(Qt::darkBlue) : params_.getColor("filtered color")));
	}
	else if (parameter=="filtered symbol size")
	{
		QScatterSeries* series = qobject_cast<QScatterSeries*>(search("filtered"));
		setSymbol(series, params_.getInt("filtered symbol size"), (color_by_column_ ? QColor(Qt::darkBlue) : params_.getColor("filtered color")));
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
			addSeries(true);
		}
		updateAxisRanges();
	}
}

void ScatterPlot::addSeries(bool filtered_out)
{
	//determine 1% noise value
	double noise_perc_x = params_.getInt("position noise") / 100.0;
	double noise_perc_y = params_.getInt("position noise") / 100.0;
	bool add_noise = noise_perc_x>0;
	if (add_noise)
	{
		QRectF bb = getBoundingBox();
		noise_perc_x *= bb.width();
		noise_perc_y *= bb.height();
	}

	//add series to chart
	QScatterSeries* series = new QScatterSeries();
	series->setName(filtered_out ? "filtered" : "visible");
	setSymbol(series, params_.getInt(filtered_out ? "filtered symbol size" : "symbol size"), (color_by_column_ ? QColor(Qt::darkBlue) : params_.getColor(filtered_out ? "filtered color" : "color")));
	QHash<int, QHash<QXYSeries::PointConfiguration, QVariant>> point_colors;
	for(int i=0; i<filter_.count(); ++i)
	{
		if (filtered_out && filter_[i]) continue;
		if (!filtered_out && !filter_[i]) continue;

		double x = values_x_.value(i);
		double y = values_y_.value(i);

		if (!BasicStatistics::isValidFloat(x) || !BasicStatistics::isValidFloat(y)) continue;

		if (add_noise)
		{
			x += Helper::randomNumber(-1,1) * noise_perc_x;
			y += Helper::randomNumber(-1,1) * noise_perc_y;
		}
		series->append(x, y);
		if (color_by_column_)
		{
			point_colors[series->count() - 1].insert(QXYSeries::PointConfiguration::Color, row_colors_.at(i));
		}
	}
	chart_->addSeries(series);
	if (color_by_column_)
	{
		series->setPointsConfiguration(point_colors);
		for (QLegendMarker* marker : chart_->legend()->markers(series)) marker->setVisible(false);
	}

	//add/attach axes
	if (chart_->axes().count()==0)
	{
		if (x_is_date_)
		{
			QDateTimeAxis* x_axis = new QDateTimeAxis();
			x_axis->setFormat("yyyy-MM-dd");
			chart_->addAxis(x_axis, Qt::AlignBottom);

			QValueAxis* y_axis = new QValueAxis();
			chart_->addAxis(y_axis, Qt::AlignLeft);

			series->attachAxis(chart_->axes(Qt::Horizontal).at(0));
			series->attachAxis(chart_->axes(Qt::Vertical).at(0));
		}
		else
		{
			chart_->createDefaultAxes();
		}
	}
	else
	{
		series->attachAxis(chart_->axes(Qt::Horizontal).at(0));
		series->attachAxis(chart_->axes(Qt::Vertical).at(0));
	}
}

void ScatterPlot::updateAxisRanges()
{
	//Use plotted coordinates so position noise is included in the range.
	double x_min = std::numeric_limits<double>::max();
	double x_max = -std::numeric_limits<double>::max();
	double y_min = x_min;
	double y_max = x_max;
	for (QAbstractSeries* abstract_series : chart_->series())
	{
		const QScatterSeries* series = qobject_cast<QScatterSeries*>(abstract_series);
		if (series==nullptr) continue;
		for (const QPointF& point : series->points())
		{
			x_min = qMin(x_min, point.x());
			x_max = qMax(x_max, point.x());
			y_min = qMin(y_min, point.y());
			y_max = qMax(y_max, point.y());
		}
	}
	if (x_min>x_max) return;

	//Give constant-valued axes a non-zero span as well.
	const double x_span = x_max>x_min ? x_max-x_min : (x_is_date_ ? 86400000.0 : qMax(1.0, qAbs(x_min)));
	const double y_span = y_max>y_min ? y_max-y_min : qMax(1.0, qAbs(y_min));
	const double x_margin = x_span * 0.01;
	const double y_margin = y_span * 0.01;
	chart_->zoomReset();
	if (x_is_date_)
	{
		QDateTimeAxis* axis = qobject_cast<QDateTimeAxis*>(chart_->axes(Qt::Horizontal).at(0));
		axis->setRange(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(floor(x_min-x_margin))),
			QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(ceil(x_max+x_margin))));
	}
	else
	{
		chart_->axes(Qt::Horizontal).at(0)->setRange(x_min-x_margin, x_max+x_margin);
	}
	chart_->axes(Qt::Vertical).at(0)->setRange(y_min-y_margin, y_max+y_margin);
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
			double x = values_x_[i];
			double y = values_y_[i];
			if (!BasicStatistics::isValidFloat(x) || !BasicStatistics::isValidFloat(y)) continue;
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
	if (series==nullptr) return;
	if (color_by_column_)
	{
		series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
		series->setMarkerSize(size);
		series->setBrush(QBrush(color));
		series->setPen(QPen(Qt::NoPen));
		return;
	}

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
