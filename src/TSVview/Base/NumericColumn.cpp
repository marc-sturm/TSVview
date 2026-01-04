#include "NumericColumn.h"
#include "CustomExceptions.h"
#include "BasicStatistics.h"
#include <algorithm>
#include <math.h>

NumericColumn::NumericColumn()
	: BaseColumn(NUMERIC)
	, values_()
    , decimals_()
    , header_()
{
}

void NumericColumn::setString(int row, const QString& value)
{
	Q_ASSERT(row<values_.count());

    auto tmp = toDouble(value);

    values_[row] = tmp.first;
    decimals_[row] = tmp.second;

	emit dataChanged();
}

StatisticsSummary NumericColumn::statistics(const QBitArray& filter) const
{
	if (filter.count()==0)
	{
		return basicStatistics(values_);
	}

	return basicStatistics(values(filter));
}

QPair<double, double> NumericColumn::getMinMax(const QBitArray& filter) const
{
	if (filter.count()==0)
	{
		return BasicStatistics::getMinMax(values_);
	}

	return BasicStatistics::getMinMax(values(filter));
}

QVector<double> NumericColumn::values(const QBitArray& filter) const
{
	QVector<double> output;
	output.reserve(filter.count(true));
	for (int i=0; i<filter.count(); ++i)
	{
		if (filter[i])
		{
            output << values_[i];
		}
	}

	return output;
}

void NumericColumn::appendString(const QString& value)
{
    auto tmp = toDouble(value);

    values_ << tmp.first;
    decimals_ << tmp.second;

	emit dataChanged();
}


void NumericColumn::sort(bool reverse)
{
    //TODO also sort decimal places!!!
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
    if (std::isnan(a)) a = std::numeric_limits<double>::max();
    if (std::isnan(b)) b = std::numeric_limits<double>::max();

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

QPair<double, char> NumericColumn::toDouble(const QString& value, bool nan_instead_of_exception)
{
    //special handling inf
    if (value=="inf" || value=="INF")
    {
        return QPair<double, char>(std::numeric_limits<double>::infinity(), 0);
    }

    // speical handling nan
    if (value=="nan" || value=="NAN")
    {
        return QPair<double, char>(std::numeric_limits<double>::quiet_NaN(), 0);
    }

    bool ok = true;
    double fvalue = value.toDouble(&ok);
    if (!ok)
    {
        if (nan_instead_of_exception) return QPair<double, char>(NAN, 0);
        THROW(Exception,"Cannot convert '" + value + "' to a number!");
    }

    char decimals = 0;
    int dot_index = value.indexOf('.');
    if (dot_index!=-1)
    {
        for (int i=dot_index; i<value.length(); ++i)
        {
            if (value[i].isDigit()) ++decimals;
        }
    }

    return QPair<double, char>(fvalue, decimals);
}
