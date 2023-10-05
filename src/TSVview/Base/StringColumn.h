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

	const QVector<QString>& values() const;
	void setValues(const QVector<QString>& values);
	const QString& value(int row) const;
	void setValue(int row, const QString& value);
	virtual void resize(int rows);
	virtual void reserve(int rows);
	virtual void sort(bool reverse = false);
	virtual int count() const;
	virtual BaseColumn* clone() const;

	virtual void setFilter(Filter filter);
	virtual void matchFilter(QBitArray& array) const;

	// See base class
	virtual QString string(int row) const;
	virtual void setString(int row, const QString& value);
	void appendString(const QString& value);

protected:
	QVector<QString> values_;
	QString header_;
};

#endif // STRINGCOLUMN_H
