#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "Histogram.h"
#include "BasePlot.h"
#include "DataSet.h"

class BoxPlot
		: public BasePlot
		, private QwtPlotItem
{
	Q_OBJECT

public:
	BoxPlot(QWidget* parent = 0);
	void setData(DataSet& data, QList<int> cols, QString filename);

protected:
	virtual int rtti() const;
	virtual void draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect) const;
	virtual void drawBox(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, int col, QColor color, QBitArray filter, int offset=0) const;

private:
	DataSet* data_;
	QList<int> cols_;

private slots:
	void replot_();
};

#endif
