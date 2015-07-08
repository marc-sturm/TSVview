#ifndef StatisticsSummaryWidget_H
#define StatisticsSummaryWidget_H

#include "ui_StatisticsSummaryWidget.h"
#include "StatisticsSummary.h"

class StatisticsSummaryWidget
		: public QWidget
{
	Q_OBJECT

public:
	StatisticsSummaryWidget(QWidget *parent = 0);
	void setData(StatisticsSummary data);

	static QString formatNumber(double number);

private:
	Ui::StatisticsSummaryWidget ui_;
};

#endif
