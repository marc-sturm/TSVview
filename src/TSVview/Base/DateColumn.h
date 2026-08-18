#ifndef DATECOLUMN_H
#define DATECOLUMN_H

#include "BaseColumn.h"
#include <QVector>
#include <QDate>

class DateColumn
		: public BaseColumn
{
	Q_OBJECT

public:
	DateColumn();

	const QVector<QDate>& values() const
    {
        return values_;
    }
	QVector<QDate> values(const QBitArray& filter) const;
	void setValues(const QVector<QDate>& values)
	{
		values_ = values;
		emit dataChanged();
	}
	const QDate& value(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row];
	}
	void setValue(int row, const QDate& value)
	{
        Q_ASSERT(row>0 && row<values_.count());
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
	virtual QVector<int> getSortOrder(bool reverse);
	virtual void reorder(const QVector<int>& order);
    virtual qsizetype count() const
	{
		return values_.count();
	}
    virtual qsizetype capacity() const
    {
        return values_.capacity();
    };
	virtual BaseColumn* clone() const
	{
		return new DateColumn(*this);
	}

	// See base class
	virtual QString string(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row].toString(Qt::ISODate);
	}
	virtual void setString(int row, const QString& value);
	void appendString(const QString& value);

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;
	static QDate toDate(const QString& value);
protected:
	QVector<QDate> values_;
    QString header_;
};

#endif // DATECOLUMN_H
