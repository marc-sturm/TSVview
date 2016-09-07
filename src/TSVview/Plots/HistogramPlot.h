#ifndef HistogramPlot_H
#define HistogramPlot_H

#include "Histogram.h"
#include "BasePlot.h"
#include "DataSet.h"
#include <QSharedPointer>

class HistogramPlot
		: public BasePlot
		, private QwtPlotItem
{
	Q_OBJECT

public:
	HistogramPlot(QWidget *parent = 0);
	void setData(DataSet& data, int column, QString filename);

protected:
	virtual int rtti() const;
	virtual void draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect) const;

private:
	DataSet* data_;
	int column_;
	bool percentage_;
	QSharedPointer<Histogram> hist_;
	QSharedPointer<Histogram> hist2_;

private slots:
	void replot_();
};

#endif
