#ifndef DATASET_H
#define DATASET_H

#include "StringColumn.h"
#include "NumericColumn.h"
#include <QSet>

/// A dataset (martix) consisting of several formatted columns (string, float).
class DataSet
		: public QObject
{
	Q_OBJECT

public:
	DataSet();
	~DataSet();

	const BaseColumn& column(int column) const
	{
		Q_ASSERT(column<columns_.size());
		return *(columns_[column]);
	}
	BaseColumn& column(int column)
	{
		Q_ASSERT(column<columns_.size());
		return *(columns_[column]);
	}
	const StringColumn& stringColumn(int column) const
	{
		Q_ASSERT(column<columns_.size());
		Q_ASSERT(columns_[column]->type()==BaseColumn::STRING);
		return *dynamic_cast<const StringColumn*>(columns_[column]);
	}

	StringColumn& stringColumn(int column)
	{
		Q_ASSERT(column<columns_.size());
		Q_ASSERT(columns_[column]->type()==BaseColumn::STRING);

		return *dynamic_cast<StringColumn*>(columns_[column]);
	}
	const NumericColumn& numericColumn(int column) const
	{
		Q_ASSERT(column<columns_.size());
		Q_ASSERT(columns_[column]->type()==BaseColumn::NUMERIC);

		return *dynamic_cast<const NumericColumn*>(columns_[column]);
	}
	NumericColumn& numericColumn(int column)
	{
		Q_ASSERT(column<columns_.size());
		Q_ASSERT(columns_[column]->type()==BaseColumn::NUMERIC);

		return *dynamic_cast<NumericColumn*>(columns_[column]);
	}

	/// Retruns the column header list.
	QStringList headers();

	/// Returns the index of the column with the given name, or -1 if no such column exists.
	int indexOf(const QString& name);

	int columnCount() const
	{
		return columns_.size();
	}
	int rowCount() const
	{
		return columns_.size()==0 ? 0 : columns_[0]->count();
	}
	void clear(bool emit_signals);
	void removeColumn(int column)
	{
		removeColumns(QSet<int>() << column);
	}
	void removeColumns(QSet<int> columns);
	void addColumn(QString header, const QVector<double>& data, bool auto_format = true, int index = -1);
	void addColumn(QString header, const QVector<QString>& data, bool auto_format = true, int index = -1);
	void replaceColumn(int index, QString header, const QVector<double>& data, bool auto_format = true);
	void sortByColumn(int column, bool reverse);
	void mergeColumns(QList<int> cols, QString header, QString sep);
	void reduceToRows(QSet<int> rows);
	void convertStringToNumeric(int c);

	bool modified() const
	{
		return modified_;
	}
	void setModified(bool changed);

	bool filtersEnabled() const
	{
		return filters_enabled_;
	}
	void setFiltersEnabled(bool enabled);
	bool filtersPresent() const;
	QBitArray getRowFilter(bool update = true) const;

	void setComments(QStringList& comments)
	{
		comments_ = comments;
	}
	QStringList comments() const
	{
		return comments_;
	}

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
	QStringList comments_;
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
