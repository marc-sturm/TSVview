#include "DateColumn.h"
#include "CustomExceptions.h"

DateColumn::DateColumn()
	: BaseColumn(DATE)
	, values_()
    , header_()
{
}

void DateColumn::setString(int row, const QString& value)
{
	Q_ASSERT(row<values_.count());

	values_[row] = toDate(value);

	emit dataChanged();
}

QVector<QDate> DateColumn::values(const QBitArray& filter) const
{
	QVector<QDate> output;
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

void DateColumn::appendString(const QString& value)
{
	values_ << toDate(value);

	emit dataChanged();
}

QVector<int> DateColumn::getSortOrder(bool reverse)
{
	//create tmp datastructure with value and index
	const int size = count();
	QVector<QPair<QDate, int>> tmp;
	tmp.reserve(size);
	for (int i=0; i<size; ++i)
	{
		QDate value = values_[i];
		if (!value.isValid()) value = reverse ? QDate(-9999, 1, 1) : QDate(9999, 1, 1);
		tmp << std::make_pair(value, i);
	}

	//sort the vector according to the value
	if (!reverse)
	{
		std::sort(tmp.begin(), tmp.end());
	}
	else
	{
		std::sort(tmp.begin(), tmp.end(), std::greater<QPair<QDate,int>>());
	}

	//create output
	QVector<int> indices;
	indices.reserve(size);
	foreach(const auto& pair, tmp)
	{
		indices << pair.second;
	}

	return indices;
}

void DateColumn::reorder(const QVector<int>& order)
{
	const int size = count();

	QVector<QDate> new_col;
	new_col.reserve(size);
	for (int i=0; i<size; ++i)
	{
		new_col << values_[order[i]];
	}
	setValues(new_col);
}

void DateColumn::setFilter(Filter filter)
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

void DateColumn::matchFilter(QBitArray& array) const
{
	Filter::Type type = filter().type();
	if (type == Filter::NONE) return;

	QDate value = QDate::fromString(filter().value(), Qt::ISODate);

	if (type == Filter::DATE_EXACT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r]==value;
			}
		}
	}
	else if (type == Filter::DATE_EXACT_NOT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r]!=value;
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
		THROW(FilterTypeException,"Internal error: Unknown date filter type!");
	}
}

QDate DateColumn::toDate(const QString& value)
{
	if (value=="") return QDate();

	QDate date = QDate::fromString(value, Qt::ISODate);
	if (!date.isValid()) THROW(Exception,"Cannot convert '" + value + "' to a date!");

	return date;
}