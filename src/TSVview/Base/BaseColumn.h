#ifndef BASECOLUMN_H
#define BASECOLUMN_H

#include "Filter.h"
#include <QObject>
#include <QString>
#include <QBitArray>

class BaseColumn
		: public QObject
{
	Q_OBJECT

public:

	enum Type
		{
		NUMERIC,
		STRING
		};

	BaseColumn(Type type);
	BaseColumn(const BaseColumn& rhs);

	Type type() const;

	/// Returns the header.
	const QString& header() const;
	/// Returns the header or the index if the header is empty. If @p force_index is true, the index is always shown.
	QString headerOrIndex(int index, bool force_index = false) const;
	///Returns @p false if the header contains invalid characters and was thus not accepted.
	bool setHeader(const QString& header);

	/// Returns a string representation of a value for displaying.
	virtual QString string(int row) const = 0;
	virtual void setString(int row, const QString& value) = 0;

	/// Automatically formats the column according to the content. The default implementation is empty.
	virtual void autoFormat();

	virtual void resize(int rows) = 0;
	virtual void reserve(int rows) = 0;
	virtual void sort(bool reverse=false) = 0;
	virtual int count() const = 0;
	virtual BaseColumn* clone() const = 0;

	Filter filter() const;
	virtual void setFilter(Filter filter) = 0;
	virtual void matchFilter(QBitArray& array) const = 0;

signals:
	void dataChanged();
	void filterChanged();
	void headerChanged();

protected:
	QString header_;
	Type type_;
	Filter filter_;
};

#endif // BASECOLUMN_H
