#include <cmath>
#include <limits>
#include <algorithm>

#include "CustomExceptions.h"
#include "Histogram.h"

Histogram::Histogram()
	: min_(0.0)
	, max_(0.0)
	, bin_size_(0.0)
	, percentage_mode_(false)
	, bin_sum_(0)
{
}

void Histogram::init(double min, double max, double bin_size, bool percentage_mode)
{
	min_ = min;
	max_ = max;
	bin_size_ = bin_size;
	percentage_mode_ = percentage_mode;
	bin_sum_ = 0;
	bins_.clear();

	if (bin_size_<=0)
	{
		THROW(StatisticsException,"Cannot initialize histogram with negative bin size!");
	}

	if (min_>=max_)
	{
		THROW(StatisticsException,"Cannot initialize histogram with empty range!");
	}

	bins_.resize(ceil((max_-min_)/bin_size_));
}

double Histogram::min() const
{
	return min_;
}

double Histogram::max() const
{
	return max_;
}

double Histogram::maxValue() const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double max = *(std::max_element(bins_.begin(), bins_.end()));
	if(percentage_mode_)
	{
		return 100.0 * max / (double)bin_sum_;
	}
	return max;
}

double Histogram::minValue() const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double min = *(std::min_element(bins_.begin(), bins_.end()));
	if(percentage_mode_)
	{
		return 100.0 * min / (double)bin_sum_;
	}
	return min;
}

double Histogram::binSize() const
{
	return bin_size_;
}

int Histogram::binCount() const
{
	return bins_.size();
}

int Histogram::binSum()
{
	return bin_sum_;
}

double Histogram::binValue(int index) const
{
	if (index<0 || index>=(int)bins_.size())
	{
		THROW(StatisticsException,"Index out of range!");
	}

	double value = bins_[index];
	if(percentage_mode_)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

double Histogram::centerOfBin(int index) const
{
	if (index<0 || index>=(int)bins_.size())
	{
		THROW(StatisticsException,"Index out of range!");
	}

	return (double)(min_+((double)index+0.5)*bin_size_);
}

double Histogram::binValue(double val) const
{
	double value = bins_[valToBin_(val)];
	if(percentage_mode_)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

void Histogram::inc(double val, bool ignore_bounds_errors)
{
	if (ignore_bounds_errors && (val < min_ || val > max_))
	{
		return;
	}

	bins_[valToBin_(val)]+=1;
	bin_sum_ += 1;
}

void Histogram::inc(QVector<double> data, bool ignore_bounds_errors)
{
	for (int i=0; i<data.size(); ++i)
	{
		inc(data[i], ignore_bounds_errors);
	}
}


int Histogram::valToBin_(double val) const
{
	if (val < min_ || val > max_)
	{
		THROW(StatisticsException,"Requested position not in range!");
	}

	int index = floor ( (val-min_) / (max_-min_) * bins_.size());
	index = std::max(0, index);
	index = std::min(index, (int)(bins_.size()-1));

	return index;
}


QString Histogram::toString() const
{
	QString output;

	for (int i=0; i<bins_.size(); ++i)
	{
		output += QString::number(centerOfBin(i), 'g', 2) + ": " + QString::number(binValue((int)i)) + "\n";
	}

	return output;
}
