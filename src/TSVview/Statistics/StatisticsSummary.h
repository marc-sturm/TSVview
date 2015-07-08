#ifndef STATISTICSSUMMARY_H
#define STATISTICSSUMMARY_H

#include <QVector>

#include "Parameters.h"

struct StatisticsSummary
{
	int count;
	int count_invalid;

	double sum;

	double min;
	double q1;
	double median;
	double q3;
	double max;

	double mean;
	double stdev;
};

StatisticsSummary basicStatistics(QVector<double> data);

#endif
