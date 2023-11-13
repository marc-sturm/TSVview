#include "StatisticsSummaryWidget.h"
#include <math.h>

StatisticsSummaryWidget::StatisticsSummaryWidget(QWidget *parent)
	: QWidget(parent)
	, ui_()
{
	ui_.setupUi(this);
}

void StatisticsSummaryWidget::setData(StatisticsSummary data)
{
	ui_.count->setText(formatNumber(data.count));
	ui_.count_invalid->setText(formatNumber(data.count_invalid));
	ui_.sum->setText(formatNumber(data.sum));

	ui_.min->setText(formatNumber(data.min));
	ui_.q1->setText(formatNumber(data.q1));
	ui_.median->setText(formatNumber(data.median));
	ui_.q3->setText(formatNumber(data.q3));
	ui_.max->setText(formatNumber(data.max));

	ui_.mean->setText(formatNumber(data.mean));
	ui_.stdev->setText(formatNumber(data.stdev));
}

QString StatisticsSummaryWidget::formatNumber(double number)
{
	//integers => no decimal places
	if (fmod(number, 1.0)==0.0)
	{
		return QString::number(number, 'f', 0);
	}

	//large numbers => 2 decical places
	if (number>10000)
	{
		return QString::number(number, 'f', 2);
	}

	return QString::number(number);
}
