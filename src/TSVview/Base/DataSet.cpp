#include "DataSet.h"
#include <math.h>
#include <algorithm>
#include <QDebug>
#include <BasicStatistics.h>

DataSet::DataSet()
	: QObject(0)
	, columns_()
	, modified_(true)
	, filters_enabled_(true)
	, filtered_rows_()
{
	columns_.reserve(100);
}


DataSet::~DataSet()
{
	clear(false);
}

void DataSet::clear(bool emit_signals)
{
	for (int i=0; i<columns_.count(); ++i)
	{
		delete(columns_[i]);
	}

	columns_.clear();

	if (emit_signals)
	{
		setModified(true);
		emit filtersChanged();
		emit headersChanged();
		emit dataChanged();
	}
}

QStringList DataSet::headers()
{
	QStringList output;
	foreach(BaseColumn* col, columns_)
	{
		output << col->header();
	}
	return output;
}

int DataSet::indexOf(const QString& name)
{
	for (int i=0; i<columns_.count(); ++i)
	{
		if (columns_[i]->header()==name)
		{
			return i;
		}
	}

	return -1;
}

void DataSet::removeColumns(QSet<int> columns)
{
	//special case: removing all columns
	if (columns.count()==columnCount())
	{
		clear(true);
		return;
	}

	//sort coumns in reverse order
	QList<int> column_list = SET_TO_LIST(columns);
	std::sort(column_list.begin(), column_list.end(), std::greater<int>());

	//remove columns
	for (int i=0; i<column_list.count(); ++i)
	{
		int col = column_list[i];
		Q_ASSERT(col<columns_.size());

		delete(columns_[col]);
		columns_.remove(col);
	}

	emit dataChanged();
	emit filtersChanged();
	setModified(true);
}

void DataSet::addColumn(QString header, const QVector<double>& data, bool auto_format, int index)
{
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());

	NumericColumn* new_col = new NumericColumn();
	new_col->setValues(data);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	if (auto_format)
	{
		new_col->autoFormat();
	}

	if (index<0 || index>=data.size())
	{
		columns_.append(new_col);
	}
	else
	{
		columns_.insert(index, new_col);
	}

	emit dataChanged();
	setModified(true);
}

void DataSet::addColumn(QString header, const QVector<QString>& data, bool auto_format, int index)
{
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());

	StringColumn* new_col = new StringColumn();
	new_col->setValues(data);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	if (auto_format)
	{
		new_col->autoFormat();
	}

	if (index<0 || index>=data.size())
	{
		columns_.append(new_col);
	}
	else
	{
		columns_.insert(index, new_col);
	}

	setModified(true);
	emit dataChanged();
}

void DataSet::replaceColumn(int index, QString header, const QVector<double>& data, bool auto_format)
{
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());
	Q_ASSERT(index < data.size());

	NumericColumn* new_col = new NumericColumn();
	new_col->setValues(data);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	if (auto_format)
	{
		new_col->autoFormat();
	}

	//replace old column
	BaseColumn* old_col = columns_[index];
	columns_.replace(index, new_col);
	delete old_col;

	emit dataChanged();
	setModified(true);
}

void DataSet::setModified(bool modified)
{
	if (modified_!=modified)
	{
		modified_ = modified;
		emit modificationStatusChanged(modified_);
	}
}

void DataSet::sortByColumn(int column, bool reverse)
{
	Q_ASSERT(column<columns_.size());

	int size = rowCount();
	QVector<int> indices(size);

	//create temporary vector with column data and index
	if (columns_[column]->type()==BaseColumn::NUMERIC)
	{
		std::vector<std::pair<double, int> > tmp;
		tmp.reserve(size);
		QVector<double> col = dynamic_cast<NumericColumn*>(columns_[column])->values();
		for (int i=0; i<size; ++i)
		{
			double value = col[i];
			if (!BasicStatistics::isValidFloat(value)) value = std::numeric_limits<double>::max();
			tmp.push_back(std::make_pair(value, i));
		}

		//sort the vector according to the value
		if (!reverse)
		{
			std::sort(tmp.begin(), tmp.end());
		}
		else
		{
			std::sort(tmp.begin(), tmp.end(), std::greater<std::pair<double, int> >());
		}

		for (int i=0; i<size; ++i)
		{
			indices[i] = tmp[i].second;
		}
	}
	else
	{
		std::vector<std::pair<QString, int> > tmp;
		tmp.reserve(size);
		QVector<QString> col = dynamic_cast<StringColumn*>(columns_[column])->values();
		for (int i=0; i<size; ++i)
		{
			tmp.push_back(std::make_pair(col[i], i));
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

		for (int i=0; i<size; ++i)
		{
			indices[i] = tmp[i].second;
		}
	}

	//use the indices to change the order of all columns
	for (int c=0; c<columns_.size(); ++c)
	{
		if (columns_[c]->type()==BaseColumn::NUMERIC)
		{
			QVector<double> col = dynamic_cast<NumericColumn*>(columns_[c])->values();
			QVector<double> new_col(size);
			for (int i=0; i<size; ++i)
			{
				new_col[i] = col[indices[i]];
			}
			dynamic_cast<NumericColumn*>(columns_[c])->setValues(new_col);
		}
		else
		{
			QVector<QString> col = dynamic_cast<StringColumn*>(columns_[c])->values();
			QVector<QString> new_col(size);
			for (int i=0; i<size; ++i)
			{
				new_col[i] = col[indices[i]];
			}
			dynamic_cast<StringColumn*>(columns_[c])->setValues(new_col);
		}
	}

	setModified(true);
	emit dataChanged();
}

void DataSet::mergeColumns(QList<int> cols, QString header, QString sep)
{
	int first_col = cols.at(0);

	//create new column
	QVector<QString> new_col;
	new_col.reserve(rowCount());
	for (int r=0; r<rowCount(); ++r)
	{
		QString value;
		foreach(int c, cols)
		{
			if (c!=first_col) value.append(sep);
			value.append(column(c).string(r));
		}
		new_col.append(value);
	}

	//remove old columns and add new one
	removeColumns(LIST_TO_SET(cols));
	addColumn(header, new_col, true, first_col);
}

void DataSet::reduceToRows(QSet<int> rows)
{
	//convert rows set to ordered list
	QList<int> keep_rows = SET_TO_LIST(rows);
	std::sort(keep_rows.begin(), keep_rows.end());

	//update all columns
	blockSignals(true);
	for (int c=0; c<columnCount(); ++c)
	{
		//numeric column
		if (column(c).type()==BaseColumn::NUMERIC)
		{
			NumericColumn& column = numericColumn(c);

			QVector<double> values;
			values.reserve(keep_rows.count());
			for (int r=0; r<keep_rows.count(); ++r)
			{
				values.append(column.value(keep_rows[r]));
			}
			column.setValues(values);
		}
		//string column
		else
		{
			StringColumn& column = stringColumn(c);

			QVector<QString> values;
			values.reserve(keep_rows.count());
			for (int r=0; r<keep_rows.count(); ++r)
			{
				values.append(column.value(keep_rows[r]));
			}
			column.setValues(values);
		}
	}
	blockSignals(false);

	emit dataChanged();
}

void DataSet::convertStringToNumeric(int c)
{
	Q_ASSERT(c>=0);
	Q_ASSERT(c<columns_.size());

	//create numeric data
	const QVector<QString> strings = stringColumn(c).values();
	QVector<double> numbers;
	numbers.reserve(strings.count());
	foreach(const QString& element, strings)
	{
		numbers << element.toDouble();
	}

	//replace string by numeric column
	replaceColumn(c, column(c).header(), numbers, false);
}

void DataSet::setFiltersEnabled(bool enabled)
{
	filters_enabled_ = enabled;

	emit filtersChanged();
}

bool DataSet::filtersPresent() const
{
	for (int i=0; i<columnCount(); ++i)
	{
		if (column(i).filter().type() != Filter::NONE)
		{
			return true;
		}
	}

	return false;
}

void DataSet::columnDataChanged()
{
	setModified(true);

	BaseColumn* column = qobject_cast<BaseColumn*>(sender());
	emit columnChanged(columns_.indexOf(column), false);
}

void DataSet::headerDataChanged()
{
	setModified(true);

	emit headersChanged();
}

void DataSet::filterDataChanged()
{
	setModified(true);

	emit filtersChanged();
}

QBitArray DataSet::getRowFilter(bool update) const
{
	if (!filters_enabled_)
	{
		filtered_rows_.fill(true, rowCount());
	}

	if (filters_enabled_ && update)
	{
		filtered_rows_.fill(true, rowCount());

		//determine rows to render
		for (int c=0; c<columnCount(); ++c)
		{
			column(c).matchFilter(filtered_rows_);
		}
	}

	return filtered_rows_;
}
