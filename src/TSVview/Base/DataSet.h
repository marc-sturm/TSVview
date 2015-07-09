#ifndef DATASET_H
#define DATASET_H

#include "StringColumn.h"
#include "NumericColumn.h"

/// A dataset (martix) consisting of several formatted columns (string, float).
class DataSet
		: public QObject
{
	Q_OBJECT

public:
	DataSet();
	~DataSet();

	const BaseColumn& column(int column) const;
	BaseColumn& column(int column);
	const StringColumn& stringColumn(int column) const;
	StringColumn& stringColumn(int column);
	const NumericColumn& numericColumn(int column) const;
	NumericColumn& numericColumn(int column);

	/// Retruns the column header list.
	QStringList headers();

	/// Returns the index of the column with the given name, or -1 if no such column exists.
	int indexOf(QString name);

	int columnCount() const;
	int rowCount() const;
	void clear(bool emit_signals);
	void removeColumn(int column);
	void removeColumns(QList<int> columns);
	void addColumn(QString header, QVector<double> data, bool auto_format = true, int index = -1);
	void addColumn(QString header, QVector<QString> data, bool auto_format = true, int index = -1);
	void replaceColumn(int index, QString header, QVector<double> data, bool auto_format = true);
	void sortByColumn(int column, bool reverse);
	void mergeColumns(QList<int> cols, QString header, QString sep);

	bool modified() const;
	void setModified(bool changed);

	bool filtersEnabled() const;
	void setFiltersEnabled(bool enabled);
	bool filtersPresent() const;
	QBitArray getRowFilter(bool update = true) const;


signals:
	void headersChanged();
	void dataChanged();
	void columnChanged(int column, bool until_end);
	void filtersChanged();
	void modificationStatusChanged(bool status);

protected slots:
	void columnDataChanged();

protected:
	QVector<BaseColumn*> columns_;
	bool modified_;
	bool filters_enabled_;
	mutable QBitArray filtered_rows_;

private:
	//not implemented
	DataSet(const DataSet& rhs);
	//not implemented
	DataSet& operator=(const DataSet& rhs);
};

#endif
