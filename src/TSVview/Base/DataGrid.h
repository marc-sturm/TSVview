#ifndef DATAGRID_H
#define DATAGRID_H

#include <QTableWidget>
#include <QBitArray>
#include <QSet>

#include "DataSet.h"

class DataGrid
		: public QTableWidget
{
	Q_OBJECT

public:

	struct SelectionInfo
	{
		bool isColumnSelection;
		bool isRowSelection;
		bool isSingleSelection;
	};

	enum FindType
		{
		FIND_EXACT,
		FIND_CONTAINS,
		FIND_START,
		FIND_END,
		FIND_REGEXP
		};

	DataGrid(QWidget *parent = 0);

	SelectionInfo selectionInfo() const;
	QList<int> selectedColumns() const;
	QList<int> selectedRows() const;

	void setData(DataSet& dataset, int preview = 0);

	QMenu* createStandardContextMenu();

	///Returns an array of coordinates (column, row)
	QVector< QPair<int, int> > findItems(QString text, Qt::CaseSensitivity case_sensitive, FindType type) const;
	static QString findTypeToString(FindType type);

	//Returns a parsable string representation of the current filter settings
	QString filtersAsString();
	//Applis the filter settings corrsponding to the given string representation
	void filtersFromString(QString filters);

signals:
	void rendered();

public slots:
	void removeSelectedColumns();
	void editFilter(int column);
	void removeFilter(int column);
	void removeAllFilters();
	void reduceToFiltered();
	///Re-renders the headers of the current dataset.
	void renderHeaders();
	///Re-renders all columns of the current dataset starting from a given column.
	void render();
	void loadFilter();
	void storeFilter();
	void deleteFilter();

protected slots:
	void copySelectionToClipboard_();
	void pasteColumn_();
	void pasteDataset_();
	void addColumn_();
	void renameColumn_();
	void mergeColumns_();
	void setColumnFormat_();
	void convertNumericNan_();
	void convertNumericSingle_();
	void convertNumericDict_();
	void sortColumn_(bool reverse = false);
	void sortByColumn_(bool reverse = false);
	void sortColumnReverse_();
	void sortByColumnReverse_();
	void editFilter_();
	void removeFilter_();
	void removeDuplicates_();
	void keepDuplicates_();
	void columnChanged(int column, bool until_end);
	void horizontalHeaderContextMenu(const QPoint&);
	void verticalHeaderContextMenu(const QPoint&);
	void editCurrentItem(QTableWidgetItem* item);

protected:
	DataSet* data_;
	int preview_;

	void keyPressEvent(QKeyEvent* event);
	void renderItem_(int row, int column, QBitArray rows_to_render = QBitArray());
	void renderColumn_(int column, QBitArray rows_to_render = QBitArray());
	static bool isNumeric_(QString string);
};

#endif
