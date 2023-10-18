#include "StringColumn.h"
#include "CustomExceptions.h"
#include <algorithm>
#include <QRegExp>

StringColumn::StringColumn()
	: BaseColumn(STRING)
	, values_()
	, header_()
{
}

void StringColumn::sort(bool reverse)
{
	if (!reverse)
	{
		std::sort(values_.begin(), values_.end());
	}
	else
	{
		std::sort(values_.begin(), values_.end(), std::greater<QString>());
	}

	emit dataChanged();
}

void StringColumn::setFilter(Filter filter)
{
	if ( filter.type()!=Filter::NONE
		 && filter.type()!=Filter::STRING_CONTAINS
		 && filter.type()!=Filter::STRING_CONTAINS_NOT
		 && filter.type()!=Filter::STRING_EXACT
		 && filter.type()!=Filter::STRING_EXACT_NOT
		 && filter.type()!=Filter::STRING_REGEXP
		 && filter.type()!=Filter::STRING_REGEXP_NOT)
	{
		THROW(FilterTypeException,"Cannot add a non-string filter to a string column!");
	}

	filter_ = filter;

	emit filterChanged();
}

void StringColumn::matchFilter(QBitArray& array) const
{
	Filter::Type type = filter().type();
	if (type == Filter::NONE)
	{
		return;
	}

	QString value = filter().value();

	if (type == Filter::STRING_EXACT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] == value;
			}
		}
	}
	else if (type == Filter::STRING_EXACT_NOT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r] != value;
			}
		}
	}
	else if (type == Filter::STRING_CONTAINS)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = values_[r].contains(value);
			}
		}
	}
	else if (type == Filter::STRING_CONTAINS_NOT)
	{
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = !values_[r].contains(value);
			}
		}
	}
	else if (type == Filter::STRING_REGEXP)
	{
		QRegExp regexp(value);
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = (regexp.indexIn(values_[r])!=-1);
			}
		}
	}
	else if (type == Filter::STRING_REGEXP_NOT)
	{
		QRegExp regexp(value);
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = (regexp.indexIn(values_[r])==-1);
			}
		}
	}
	else
	{
		THROW(FilterTypeException,"Internal error: Unknown filter type!");
	}
}

