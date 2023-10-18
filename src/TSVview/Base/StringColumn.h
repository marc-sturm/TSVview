#ifndef STRINGCOLUMN_H
#define STRINGCOLUMN_H

#include "BaseColumn.h"
#include <QVector>

class StringColumn
		: public BaseColumn
{
	Q_OBJECT

public:
	StringColumn();

	const QVector<QString>& values() const
	{
		return values_;
	}
	void setValues(const QVector<QString>& values)
	{
		values_ = values;
		emit dataChanged();
	}
	const QString& value(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row];
	}
	void setValue(int row, const QString& value)
	{
		Q_ASSERT(row<values_.count());
		values_[row] = value;
		emit dataChanged();
	}
	virtual void resize(int rows)
	{
		values_.resize(rows);
		emit dataChanged();
	}
	virtual void reserve(int rows)
	{
		values_.reserve(rows);
	}
	virtual void sort(bool reverse = false);
	virtual int count() const
	{
		return values_.count();
	}
	virtual BaseColumn* clone() const
	{
		return new StringColumn(*this);
	}

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;

	// See base class
	virtual QString string(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row];
	}
	virtual void setString(int row, const QString& value)
	{
		Q_ASSERT(row<values_.count());
		values_[row] = value;
		emit dataChanged();
	}
	void appendString(const QString& value)
	{
		values_ << value;
		emit dataChanged();
	}


protected:
	QVector<QString> values_;
	QString header_;
};

#endif // STRINGCOLUMN_H
