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

	//sort data and calculate five number summary
	std::sort(sorted.begin(), sorted.end());

	output.min = sorted.first();
	output.max = sorted.last();
	if (output.count%2==0)
	{
		output.median = (sorted.at(output.count/2) + sorted.at(output.count/2 - 1))/2;
	}
	else
	{
		output.median = sorted.at(output.count/2);
	}

	if (output.count%4==0)
	{
		output.q1 = (sorted.at(output.count/4) + sorted.at(output.count/4 - 1))/2;
		output.q3 = (sorted.at(3*output.count/4) + sorted.at(3*output.count/4 - 1))/2;
	}
	else
	{
		output.q1 = sorted.at(output.count/4);
		output.q3 = sorted.at(3*output.count/4);
	}

	// calculate mean
	output.mean = BasicStatistics::mean(sorted);

	// calculate stdev
	output.stdev = BasicStatistics::stdev(sorted, output.mean);

	return output;
}
