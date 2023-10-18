#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "Histogram.h"
#include "BasePlot.h"
#include "DataSet.h"

class BoxPlot
		: public BasePlot
{
	Q_OBJECT

public:
	BoxPlot(QWidget* parent = 0);
	void setData(DataSet& data, QList<int> cols, QString filename);

protected slots:
	void plot();

private:
	DataSet* data_;
	QList<int> cols_;
};

#endif
