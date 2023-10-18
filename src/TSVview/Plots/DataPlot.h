#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "BasePlot.h"
#include "DataSet.h"

class DataPlot
		: public BasePlot
{
	Q_OBJECT

public:
	DataPlot(QWidget *parent = 0);
	void setData(const DataSet& data, QList<int> cols, QString filename);

private:
	QPen pen(QString series_name);
	bool pointsVisible(QString series_name);

private slots:
	void parameterChanged(QString parameter);
};

#endif
