#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include "BasePlot.h"
#include "DataSet.h"
#include <QScatterSeries>

class ScatterPlot
		: public BasePlot
{
	Q_OBJECT

public:
	ScatterPlot(QWidget* parent = 0);
	void setData(const DataSet& data, int col1, int col2, QString filename, int color_col=-1);

private slots:
	void parameterChanged(QString parameter);
	//Adds points as a new series. If filtered_out==true, invisible rows are added, otherwise the visible rows are added.
	void addSeries(bool filtered_out=false);

protected:
	QBitArray filter_;
	QVector<double> values_x_;
	QVector<double> values_y_;
	bool x_is_date_;
	bool color_by_column_;
	QVector<QColor> row_colors_;

	void updateAxisRanges();
	QRectF getBoundingBox() const;
	void setSymbol(QScatterSeries* series, int size, QColor color);
};

#endif
