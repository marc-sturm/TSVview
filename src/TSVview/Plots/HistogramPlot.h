#ifndef HistogramPlot_H
#define HistogramPlot_H

#include "Histogram.h"
#include "BasePlot.h"
#include "DataSet.h"
#include <QSharedPointer>

class HistogramPlot
		: public BasePlot
{
	Q_OBJECT

public:
	HistogramPlot(QWidget *parent = 0);
	void setData(DataSet& data, int column, QString filename);

protected slots:
	void parameterChanged(QString parameter);

protected:
	QBitArray filter_;
	QVector<double> col_;
	QString name_;

	void plot();
	void addSeries();
	QPair<double, double> getMinMax();
};

#endif
