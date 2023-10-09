#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include "BasePlot.h"
#include "DataSet.h"

class ScatterPlot
		: public BasePlot
{
	Q_OBJECT

public:
	ScatterPlot(QWidget* parent = 0);
	void setData(const DataSet& data, int col1, int col2, QString filename);

private slots:
	void parameterChanged(QString parameter);
	void addSeries();
	void addSeriesFiltered();

protected:
	QBitArray filter_;
	QVector<double> col1_;
	QVector<double> col2_;

	QRectF getBoundingBox() const;
};

#endif
