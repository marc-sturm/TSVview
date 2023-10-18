#ifndef NUMERICCOLUMN_H
#define NUMERICCOLUMN_H

#include "BaseColumn.h"
#include "StatisticsSummaryWidget.h"
#include <QVector>

class NumericColumn
		: public BaseColumn
{
	Q_OBJECT

public:
	NumericColumn();

	QVector<double> values(QBitArray filter = QBitArray()) const;
	void setValues(const QVector<double>& values)
	{
		values_ = values;
		emit dataChanged();
	}
	double value(int row) const
	{
		Q_ASSERT(row<values_.count());
		return values_[row];
	}
	void setValue(int row, double value)
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
	virtual void sort(bool reverse=false);
	virtual int count() const
	{
		return values_.count();
	}
	virtual BaseColumn* clone() const
	{
		return new NumericColumn(*this);
	}

	// See base class
	virtual QString string(int row) const
	{
		Q_ASSERT(row<values_.count());
		return QString::number(values_[row], format_, decimal_places_);
	}
	virtual void setString(int row, const QString& value);
	void appendString(const QString& value);

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;

	virtual void autoFormat();
	int decimalPlaces() const
	{
		return decimal_places_;
	}
	char format() const
	{
		return format_;
	}
	void setFormat(char format, int decimal_places)
	{
		format_ = format;
		decimal_places_ = decimal_places;
		emit dataChanged();
	}

	StatisticsSummary statistics(QBitArray filter) const;
	QPair<double, double> getMinMax(QBitArray filter) const;

protected:
	QVector<double> values_;
	QString header_;
	char format_;
	int decimal_places_;

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
