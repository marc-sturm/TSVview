#include "StringColumn.h"
#include "CustomExceptions.h"
#include <algorithm>
#include <QRegularExpression>

StringColumn::StringColumn()
	: BaseColumn(STRING)
	, values_()
	, header_()
{
}

QVector<int> StringColumn::getSortOrder(bool reverse)
{
	const int size = count();

	//crete tmp datastructure with value and index
	QVector<QPair<QString, int>> tmp;
	tmp.reserve(size);
	for (int i=0; i<size; ++i)
	{
		tmp << std::make_pair(values_[i], i);
	}

	//sort the vector according to the value
	if (!reverse)
	{
		std::sort(tmp.begin(), tmp.end());
	}
	else
	{
		std::sort(tmp.begin(), tmp.end(), std::greater<std::pair<QString, int> >());
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

void StringColumn::reorder(const QVector<int>& order)
{
	const int size = count();
	Q_ASSERT(size==order.count());

	QVector<QString> new_col;
	new_col.reserve(size);
	for (int i=0; i<size; ++i)
	{
		new_col << values_[order[i]];
	}
	setValues(new_col);
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
		QRegularExpression regexp(value);
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = regexp.match(values_[r]).hasMatch();
			}
		}
	}
	else if (type == Filter::STRING_REGEXP_NOT)
	{
		QRegularExpression regexp(value);
		for (int r=0; r<count(); ++r)
		{
			if (array[r])
			{
				array[r] = !regexp.match(values_[r]).hasMatch();
			}
		}
	}
	else
	{
		THROW(FilterTypeException,"Internal error: Unknown filter type!");
	}
}

