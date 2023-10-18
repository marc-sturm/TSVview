#include "NumericColumn.h"
#include "CustomExceptions.h"
#include "BasicStatistics.h"
#include <algorithm>
#include <math.h>

NumericColumn::NumericColumn()
	: BaseColumn(NUMERIC)
	, values_()
	, header_()
	,	format_('g')
	,	decimal_places_(6)
{
}

void NumericColumn::setString(int row, const QString& value)
{
	Q_ASSERT(row<values_.count());

	bool ok = true;
	double fvalue = value.toDouble(&ok);
	if (!ok)
	{
		THROW(Exception,"Cannot convert '" + value + "' to a number!");
	}

	values_[row] = fvalue;

	emit dataChanged();
}

StatisticsSummary NumericColumn::statistics(QBitArray filter) const
{
	if (filter.count()==0)
	{
		return basicStatistics(values_);
	}

	return basicStatistics(values(filter));
}

QPair<double, double> NumericColumn::getMinMax(QBitArray filter) const
{
	if (filter.count()==0)
	{
		return BasicStatistics::getMinMax(values_);
	}

	return BasicStatistics::getMinMax(values(filter));
}

QVector<double> NumericColumn::values(QBitArray filter) const
{
	//not filtered
	if (filter.count()==0)
	{
		return values_;
	}

	//filtered
	QVector<double> output;
	output.reserve(filter.count(true));

	for (int i=0; i<filter.count(); ++i)
	{
		if (filter[i])
		{
			output.append(values_[i]);
		}
	}

	return output;
}

void NumericColumn::appendString(const QString& value)
{
	bool ok = true;
	double fvalue = value.toDouble(&ok);
	if (!ok) THROW(Exception,"Cannot convert '" + value + "' to a number!");

	values_.append(fvalue);

	emit dataChanged();
}


void NumericColumn::sort(bool reverse)
{
	if (!reverse)
	{
		std::sort(values_.begin(), values_.end(), NanAwareDoubleComp());
	}
	else
	{
		std::sort(values_.begin(), values_.end(), NanAwareDoubleComp(true));
	}

	emit dataChanged();
}

void NumericColumn::autoFormat()
{
	bool is_int = true;
	for (int i=0; i<values_.count(); ++i)
	{
		double rest = fmod(values_[i], 1.0);
		if (rest!=0.0)
		{
			is_int = false;
			break;
		}
	}

	if (is_int)
	{
		format_ = 'f';
		decimal_places_ = 0;
	}
}

void NumericColumn::setFilter(Filter filter)
{
	if ( filter.type()!=Filter::NONE
		 && filter.type()!=Filter::FLOAT_EXACT
		 && filter.type()!=Filter::FLOAT_EXACT_NOT
		 && filter.type()!=Filter::FLOAT_GREATER
		 && filter.type()!=Filter::FLOAT_GREATER_EQUAL
		 && filter.type()!=Filter::FLOAT_LESS
		 && filter.type()!=Filter::FLOAT_LESS_EQUAL
		 )
	{
		THROW(FilterTypeException,"Cannot add a non-numeric filter to a numeric column!");
	}

	filter_ = filter;

	emit filterChanged();
}

void NumericColumn::matchFilter(QBitArray &array) const
{
	Filter::Type type = filter().type();
	if (type == Filter::NONE)
	{
		return;
	}

	double value = filter().value().toFloat();

	if (type == Filter::FLOAT_EXACT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = fabs(values_[r] - value) < 0.0001;
			}
		}
	}
	else if (type == Filter::FLOAT_EXACT_NOT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = fabs(values_[r] - value) > 0.0001;
			}
		}
	}
	else if (type == Filter::FLOAT_GREATER)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] > value;
			}
		}
	}
	else if (type == Filter::FLOAT_GREATER_EQUAL)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] >= value;
			}
		}
	}
	else if (type == Filter::FLOAT_LESS)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] < value;
			}
		}
	}
	else if (type == Filter::FLOAT_LESS_EQUAL)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] <= value;
			}
		}
	}
	else
	{
		THROW(FilterTypeException,"Internal error: Unknown filter type!");
	}
}


NumericColumn::NanAwareDoubleComp::NanAwareDoubleComp(bool greater)
	: greater_(greater)
{
}

bool NumericColumn::NanAwareDoubleComp::operator()(double a, double b) const
{
	//handle NAN
	if (a!=a) a = std::numeric_limits<double>::max();
	if (b!=b) b = std::numeric_limits<double>::max();

	//compare
	if (greater_)
	{
		if (a>b) return true;
	}
	else
	{
		if (a<b) return true;
	}

	return false;
}
