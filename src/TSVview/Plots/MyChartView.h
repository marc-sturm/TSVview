#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QChartView>
#include <QLegendMarker>

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
	void resetZoom();
	void zoomIn();

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
	 void mouseMoveEvent(QMouseEvent* event) override;
	 void keyPressEvent(QKeyEvent* event) override;
	 void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
	bool mouse_tracking_ = false;
};

#endif // MYCHARTVIEW_H
