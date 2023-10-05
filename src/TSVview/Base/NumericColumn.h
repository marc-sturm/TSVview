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
	void setValues(const QVector<double>& values);
	double value(int row) const;
	void setValue(int row, double value);
	virtual void resize(int rows);
	virtual void reserve(int rows);
	virtual void sort(bool reverse=false);
	virtual int count() const;
	virtual BaseColumn* clone() const;

	// See base class
	virtual QString string(int row) const;
	virtual void setString(int row, const QString& value);
	void appendString(const QString& value);

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;

	virtual void autoFormat();
	int decimalPlaces() const;
	char format() const;
	void setFormat(char format, int decimal_places);

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
