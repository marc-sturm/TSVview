#include <algorithm>
#include <numeric>
#include <math.h>

#include "StatisticsSummary.h"
#include "BasicStatistics.h"

StatisticsSummary basicStatistics(QVector<double> data)
{
	StatisticsSummary output;
	output.count_invalid = 0;

	//create new vector with only valid numbers; count valid and invalid values
	QVector<double> sorted;
	sorted.reserve(data.count());
	for (int i=0; i<data.count(); ++i)
	{
		if (BasicStatistics::isValidFloat(data[i]))
		{
			sorted.append(data[i]);
		}
		else
		{
			++output.count_invalid;
		}
	}
	output.count = sorted.count();

	//calculate sum
	output.sum = 0.0;
	foreach(const double& value, sorted)
	{
		output.sum += value;
	}

	//five number summary
	std::sort(sorted.begin(), sorted.end());
	output.min = sorted.first();
	output.q1 = BasicStatistics::q1(sorted, false);
	output.median = BasicStatistics::median(sorted, false);
	output.q3 = BasicStatistics::q3(sorted, false);
	output.max = sorted.last();

	//mean/stdev
	output.mean = BasicStatistics::mean(sorted);
	output.stdev = BasicStatistics::stdev(sorted, output.mean);

	return output;
}
