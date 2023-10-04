#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QChartView>

using namespace QtCharts;

class MyChartView
	: public QChartView
{
	Q_OBJECT
public:
	MyChartView(QWidget* parent = nullptr);
	void enableMouseTracking(); //activates xPosition and yPosition signals

signals:
	void xPosition(QString);
	void yPosition(QString);

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
	 void mouseMoveEvent(QMouseEvent* event) override;

private:
	bool mouse_tracking_ = false;
};

#endif // MYCHARTVIEW_H
