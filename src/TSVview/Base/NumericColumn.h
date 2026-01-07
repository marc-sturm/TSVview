#ifndef NUMERICCOLUMN_H
#define NUMERICCOLUMN_H

#include "BaseColumn.h"
#include "StatisticsSummary.h"
#include <QVector>

class NumericColumn
		: public BaseColumn
{
	Q_OBJECT

public:
	NumericColumn();

    const QVector<double>& values() const
    {
        return values_;
    }
    QVector<double> values(const QBitArray& filter) const;
    void setValues(const QVector<double>& values, const QVector<char>& decimals)
	{
        Q_ASSERT(values_.count()==decimals.count());
		values_ = values;
        decimals_ = decimals;
		emit dataChanged();
	}
	double value(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row];
	}
    void setValue(int row, double value, char decimals=-1)
	{
        Q_ASSERT(row>0 && row<values_.count());
		values_[row] = value;
        if (decimals>=0) decimals_[row] = decimals;
		emit dataChanged();
    }
    const QVector<char>& decimals() const
    {
        return decimals_;
    }
    void setDecimals(QVector<char> decimals)
    {
        Q_ASSERT(values_.count()==decimals.count());
        decimals_ = decimals;
        emit dataChanged();
    }
    char decimals(int row) const
    {
        Q_ASSERT(row<decimals_.count());
        return decimals_[row];
    }
	virtual void resize(int rows)
	{
		values_.resize(rows);
        decimals_.resize(rows);
		emit dataChanged();
	}
	virtual void reserve(int rows)
	{
		values_.reserve(rows);
        decimals_.reserve(rows);
	}
	virtual void sort(bool reverse=false);
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
		return new NumericColumn(*this);
	}

	// See base class
	virtual QString string(int row) const
	{
		Q_ASSERT(row<values_.count());
        return QString::number(values_[row], 'f', decimals_[row]);
	}
	virtual void setString(int row, const QString& value);
	void appendString(const QString& value);

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;

    StatisticsSummary statistics(const QBitArray& filter) const;
    QPair<double, double> getMinMax(const QBitArray& filter) const;


    //returns numeric value and decimal places. Throws an exception if value is not numeric, or NAN if nan_instead_of_exception=true.
    static QPair<double, char> toDouble(const QString& value, bool nan_instead_of_exception=false);

protected:
	QVector<double> values_;
    QVector<char> decimals_;
    QString header_;

	///NAN-aware comparator class for doubles. NAN is handled as numeric_limits<double>::max()
	class NanAwareDoubleComp
	{
		public:
			NanAwareDoubleComp(bool greater = false);
			bool operator()(double a, double b) const;

		private:
			bool greater_;
	};
};

#endif // NUMERICCOLUMN_H
