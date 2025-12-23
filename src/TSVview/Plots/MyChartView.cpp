#include "MyChartView.h"
#include <QDebug>

MyChartView::MyChartView(QWidget* parent)
	: QChartView(parent)
{
}

void MyChartView::enableMouseTracking()
{
	mouse_tracking_ = true;
	installEventFilter(this);
}

void MyChartView::mouseMoveEvent(QMouseEvent* event)
{
	if (mouse_tracking_)
	{
		//convert coordinates
		QPointF widget_pos = event->position();
		QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
		QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
		QPointF pos = chart()->mapToValue(chart_item_pos);

		emit xPosition("x:"+QString::number(pos.x()));
		emit yPosition("y:"+QString::number(pos.y()));
	}
	QChartView::mouseMoveEvent(event);
}

void MyChartView::keyPressEvent(QKeyEvent* event)
{
	if (event->key()==Qt::Key_Space)
	{
		emit resetZoom();
		event->accept();
		return;
	}

	QChartView::keyPressEvent(event);
}

void MyChartView::mouseDoubleClickEvent(QMouseEvent* event)
{
	QPoint global_pos = mapToGlobal(event->pos());
	QRect global_legend_rect(mapToGlobal(chart()->legend()->pos().toPoint()), chart()->legend()->size().toSize());
	if (event->button()==Qt::LeftButton && !global_legend_rect.contains(global_pos))
	{
		emit zoomIn();
		event->accept();
		return;
	}

	QChartView::mouseDoubleClickEvent(event);
}

bool MyChartView::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Leave)
	{
		emit xPosition("");
		emit yPosition("");

		return true;
	}

	return QObject::eventFilter(obj, event);
}
