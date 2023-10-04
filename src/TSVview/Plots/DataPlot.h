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
	void setData(DataSet& data, QList<int> cols, QString filename);

private:
	QPen pen(QString series_name);
	bool pointsVisible(QString series_name);
	static QColor getColor_(int i);
	static QString getLineType_(int i);

private slots:
	void parameterChanged(QString parameter);
	void legendChecked_(const QVariant& info, bool on, int index);
};

#endif
