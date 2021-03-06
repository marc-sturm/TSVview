#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <qwt_plot_curve.h>
#include <qwt_series_data.h>

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
	QList<QwtPlotCurve*> curves_;
	static QColor getColor_(int i);
	static QString getLineType_(int i);

private slots:
	void replot_();
	void legendChecked_(const QVariant& info, bool on, int index);
};

#endif
