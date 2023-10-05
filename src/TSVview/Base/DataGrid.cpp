#include <QMenu>
#include <QInputDialog>
#include <QJSEngine>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QHeaderView>
#include <QTextStream>
#include <QDebug>
#include <QTime>
#include <QRegExp>
#include <algorithm>
#include <math.h>

#include "DataGrid.h"
#include "CustomExceptions.h"
#include "TextFile.h"
#include "Settings.h"
#include "TextImportPreview.h"
#include "FilterDialog.h"
#include "ReplacementDialog.h"
#include "MergeDialog.h"
#include "GUIHelper.h"
#include "AddColumnDialog.h"
#include "TextItemEditDialog.h"

DataGrid::DataGrid(QWidget* parent)
	: QTableWidget(parent)
	, data_(0)
	, preview_(0)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	setSelectionBehavior(QAbstractItemView::SelectItems);

	QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	policy.setHorizontalStretch(5);
	policy.setVerticalStretch(5);
	setSizePolicy(policy);

	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(horizontalHeaderContextMenu(const QPoint&)));
	verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(verticalHeaderContextMenu(const QPoint&)));

	setAlternatingRowColors(true);

	//make sure the selection is visible when the table looses focus
	QString fg = GUIHelper::colorToQssFormat(palette().color(QPalette::Active, QPalette::HighlightedText));
	QString bg = GUIHelper::colorToQssFormat(palette().color(QPalette::Active, QPalette::Highlight));
	setStyleSheet(QString("QTableWidget:!active { selection-color: %1; selection-background-color: %2; }").arg(fg).arg(bg));

	connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(editCurrentItem(QTableWidgetItem*)));
}

void DataGrid::horizontalHeaderContextMenu(const QPoint& pos)
{
	if (selectionInfo().isColumnSelection)
	{
		QPoint new_pos = horizontalHeader()->mapToParent(pos);
		emit customContextMenuRequested(viewport()->mapFromParent(new_pos));
	}
}

void DataGrid::verticalHeaderContextMenu(const QPoint& pos)
{
	if (selectionInfo().isRowSelection)
	{
		QPoint new_pos = verticalHeader()->mapToParent(pos);
		emit customContextMenuRequested(viewport()->mapFromParent(new_pos));
	}
}

DataGrid::SelectionInfo DataGrid::selectionInfo() const
{
	QList<QTableWidgetSelectionRange> ranges = selectedRanges();

	//single selection
	SelectionInfo output;
	output.isSingleSelection = ranges.count() == 1;

	//column/row selection
	output.isColumnSelection = true;
	output.isRowSelection = true;
	for(int i=0; i<ranges.count(); ++i)
	{
		if (ranges[i].rowCount()!=rowCount())
		{
			output.isColumnSelection = false;
		}
		if (ranges[i].columnCount()!=columnCount())
		{
			output.isRowSelection = false;
		}
	}

	return output;
}

QList<int> DataGrid::selectedColumns() const
{
	QList<int> cols;

	QList<QTableWidgetSelectionRange> ranges = selectedRanges();
	for(int i=0; i<ranges.count(); ++i)
	{
		for(int j=ranges[i].leftColumn(); j<=ranges[i].rightColumn(); ++j)
		{
			cols.append(j);
		}
	}

	return cols;
}


QList<int> DataGrid::selectedRows() const
{
	QList<int> rows;

	QList<QTableWidgetSelectionRange> ranges = selectedRanges();
	for(int i=0; i<ranges.count(); ++i)
	{
		for(int j=ranges[i].topRow(); j<=ranges[i].bottomRow(); ++j)
		{
			rows.append(j);
		}
	}

	return rows;
}

void DataGrid::setData(DataSet& dataset, int preview)
{
	data_ = &dataset;
	preview_ = preview;

	connect(data_, SIGNAL(headersChanged()), this, SLOT(renderHeaders()));
	connect(data_, SIGNAL(columnChanged(int, bool)), this, SLOT(columnChanged(int, bool)));
	connect(data_, SIGNAL(dataChanged()), this, SLOT(render()));

	render();
}

void DataGrid::renderColumn_(int column, QBitArray rows_to_render)
{
	const BaseColumn& col = data_->column(column);
	Q_ASSERT(rows_to_render.count()==col.count());

	int r_filtered = 0;
	for (int r=0; r<data_->rowCount(); ++r)
	{
		if (rows_to_render[r])
		{
			QTableWidgetItem* item = new QTableWidgetItem(col.string(r));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			setItem(r_filtered, column, item);

			++r_filtered;
		}
	}
}

void DataGrid::renderItem_(int row, int column, QBitArray rows_to_render)
{
	Q_ASSERT(rows_to_render.count()==data_->column(column).count());

	int r_filtered = -1;
	for (int r=0; r<data_->rowCount(); ++r)
	{
		if (rows_to_render[r])
		{
			++r_filtered;
		}

		if (r_filtered==row)
		{
			QTableWidgetItem* item = new QTableWidgetItem(data_->column(column).string(r));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			setItem(row, column, item);

			break;
		}
	}
}

void DataGrid::render(int start_col)
{
	//abort if dataset is not set
	if (data_==0)
	{
		clear();
		return;
	}

	QTime timer;
	timer.start();

	// get row/column count
	int cols = data_->columnCount();
	int rows = data_->rowCount();

	//determine number of rows to render (if we need to filter)
	QBitArray rows_to_render = data_->getRowFilter();
	rows = rows_to_render.count(true);

	//set table to new dimensions
	if (preview_>0)
	{
		rows = std::min(rows, preview_);
	}

	//render
	setColumnCount(cols);
	setRowCount(rows);
	renderHeaders();
	for (int c=start_col; c<cols; ++c)
	{
		renderColumn_(c, rows_to_render);
	}

	//add preview signs (if required)
	if (preview_>0 && rows==preview_)
	{
		for (int c=0; c<cols; ++c)
		{
			setItem(preview_-1, c, new QTableWidgetItem("..."));
		}
	}

	qDebug() << "rendering table: " << cols << "/" << rows << " (" << timer.elapsed() << "ms)";

	emit rendered();
}

QMenu* DataGrid::createStandardContextMenu()
{
	QAction* action = 0;
	QMenu* menu = new QMenu();

	SelectionInfo info = selectionInfo();
	if (info.isColumnSelection)
	{
		QList<int> selected = selectedColumns();
		int selected_count = selected.size();
		int text_count = 0;
		for (int i=0; i<selected.size(); ++i)
		{
			text_count += (data_->column(selected[i]).type()==BaseColumn::STRING);
		}

		action = menu->addAction(QIcon(":/Icons/Copy.png"), "Copy column(s)", this, SLOT(copySelectionToClipboard_()));
		action->setEnabled(selected_count>0);
		action = menu->addAction(QIcon(":/Icons/Paste.png"), "Paste column(s)", this, SLOT(pasteColumn_()));
		action->setEnabled(data_!=0);
		action = menu->addAction(QIcon(":/Icons/Remove.png"), "Remove column(s)", this, SLOT(removeSelectedColumns()));
		action->setEnabled(selected_count>0);
		action = menu->addAction(QIcon(":/Icons/Add.png"), "Add column", this, SLOT(addColumn_()));
		action->setEnabled(data_!=0 && data_->columnCount()!=0);

		menu->addSeparator();
		QMenu* edit_menu = menu->addMenu("Edit");
		action = edit_menu->addAction(QIcon(":/Icons/Rename.png"), "Rename", this, SLOT(renameColumn_()));
		action->setEnabled(selected_count==1);
		action = edit_menu->addAction(QIcon(":/Icons/Merge.png"), "Merge", this, SLOT(mergeColumns_()));
		action->setEnabled(selected_count>1);
		action = edit_menu->addAction("Set number format", this, SLOT(setColumnFormat_()));
		action->setEnabled(selected_count==1 && text_count==0);
		action = edit_menu->addAction("Remove duplicates", this, SLOT(removeDuplicates_()));
		action = edit_menu->addAction("Keep duplicates", this, SLOT(keepDuplicates_()));
		action->setEnabled(selected_count==1);

		QMenu* convert_menu = menu->addMenu("Convert to numeric column");
		convert_menu->setEnabled(selected_count==1 && text_count==1);
		action = convert_menu->addAction("'nan' if fails", this, SLOT(convertNumericNan_()));
		action = convert_menu->addAction("Single value if fails", this, SLOT(convertNumericSingle_()));
		action = convert_menu->addAction("By dictionary", this, SLOT(convertNumericDict_()));

		QMenu* sort_menu = menu->addMenu(QIcon(":/Icons/Sort.png"), "Sort");
		sort_menu->setEnabled(selected_count==1);
		action = sort_menu->addAction("All columns (asc)", this, SLOT(sortByColumn_()));
		action = sort_menu->addAction("All columns (desc)", this, SLOT(sortByColumnReverse_()));
		sort_menu->addSeparator();
		action = sort_menu->addAction("Single column (asc)", this, SLOT(sortColumn_()));
		action = sort_menu->addAction("Single column (desc)", this, SLOT(sortColumnReverse_()));

		action = menu->addAction(QIcon(":/Icons/Filter.png"), "Filter", this, SLOT(editFilter_()));
		action->setEnabled(selected_count==1);
	}
	else if (info.isRowSelection)
	{
		action = menu->addAction(QIcon(":/Icons/Copy.png"), "Copy rows(s)", this, SLOT(copySelectionToClipboard_()));
	}
	else if (info.isSingleSelection)
	{
		action = menu->addAction(QIcon(":/Icons/Copy.png"), "Copy cell(s)", this, SLOT(copySelectionToClipboard_()));
	}

	return menu;
}

void DataGrid::removeSelectedColumns()
{
	if (!selectionInfo().isColumnSelection)
	{
		return;
	}

	if (QMessageBox::question(this, "Confirm delete columns", "Do you want to delete the selected columns?", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes)==QMessageBox::Cancel)
	{
		return;
	}

	data_->removeColumns(selectedColumns().toSet());
}

void DataGrid::renameColumn_()
{
	int column = selectedColumns()[0];
	QString header = data_->column(column).header();
	bool ok = true;
	header = QInputDialog::getText(this, "Set column header", "Header:", QLineEdit::Normal, header, &ok);

	if (!ok) return;

	if (!data_->column(column).setHeader(header))
	{
		QMessageBox::warning(this, "Renaming error", "The column header '" + header + "' contains invalid characters.");
	}
}

void DataGrid::mergeColumns_()
{
	QList<int> columns = selectedColumns();

	//create proposed header
	QString header;
	foreach(int c, columns)
	{
		if (header!="") header.append("_");
		header.append(data_->column(c).header());
	}

	MergeDialog dialog(header, this);
	if (!dialog.exec()) return;

	//merge columns
	data_->mergeColumns(columns, dialog.header(), dialog.separator());
}

void DataGrid::setColumnFormat_()
{
	int col_index = selectedColumns()[0];
	NumericColumn& col = data_->numericColumn(col_index);

	// get new format
	QStringList items;
	items << "auto" << "normal" << "scientific";

	int current = 0;
	if (col.format()=='f')
	{
		current = 1;
	}
	else if (col.format()=='e')
	{
		current = 2;
	}

	bool ok = true;
	QString format = QInputDialog::getItem(this, "Format", "Select format:", items, current, false, &ok);
	if (!ok)
	{
		return;
	}

	// get precision
	ok = true;
	int precision = QInputDialog::getInt(this, "Precision", "Select decimal places:", col.decimalPlaces(), 0, 20, 1, &ok);
	if (!ok)
	{
		return;
	}

	// update data and view
	if (format=="auto")
	{
		col.setFormat('g', precision);
	}
	else if (format=="normal")
	{
		col.setFormat('f', precision);
	}
	else
	{
		col.setFormat('e', precision);
	}
}

void DataGrid::convertNumericNan_()
{
	//convert
	int col_index = selectedColumns()[0];
	QVector<QString> data = data_->stringColumn(col_index).values();
	QVector<double> new_data;
	new_data.resize(data.count());
	for (int i=0; i<data.count(); ++i)
	{
		bool ok = true;
		double value = data[i].toDouble(&ok);
		if (ok)
		{
			new_data[i] = value;
		}
		else
		{
			new_data[i] = NAN;
		}
	}

	//replace column
	QString header = data_->column(col_index).header();
	data_->replaceColumn(col_index, header, new_data, true);
}


void DataGrid::convertNumericSingle_()
{
	//get fallback value
	bool ok = true;
	QString fallback_str = QInputDialog::getText(this, "Fallback value", "value", QLineEdit::Normal, "nan", &ok);
	if (!ok) return;

	double fallback_value = fallback_str.toDouble(&ok);
	if (!ok)
	{
		QMessageBox::warning(this, "Error", "Could not convert '" + fallback_str + "' to float!");
		return;
	}

	//convert
	int col_index = selectedColumns()[0];
	QVector<QString> data = data_->stringColumn(col_index).values();
	QVector<double> new_data;
	new_data.resize(data.count());
	for (int i=0; i<data.count(); ++i)
	{
		ok = true;
		double value = data[i].toDouble(&ok);
		if (ok)
		{
			new_data[i] = value;
		}
		else
		{
			new_data[i] = fallback_value;
		}
	}

	//replace column
	QString header = data_->column(col_index).header();
	data_->replaceColumn(col_index, header, new_data, true);
}


void DataGrid::convertNumericDict_()
{
	int col_index = selectedColumns()[0];

	//create list of not-convertable values
	int max_count = 20;
	QSet<QString> not_convertable;
	QVector<QString> data = data_->stringColumn(col_index).values();
	for (int i=0; i<data.count(); ++i)
	{
		if (!isNumeric_(data[i]))
		{
			not_convertable.insert(data[i]);

			if (not_convertable.count() > max_count) break;
		}
	}

	//abort if too many entries
	if (not_convertable.count() > max_count)
	{
		QMessageBox::warning(this, "Error", "Cannot convert a column with more than " + QString::number(max_count) + " distinct string values.");
		return;
	}

	//show dialog to get replacement map
	ReplacementDialog* dialog = new ReplacementDialog(this);
	dialog->setKeys(not_convertable);

	if (dialog->exec())
	{
		QMap<QString, double> map = dialog->getMap();

		//create new column
		QVector<double> new_data;
		new_data.reserve(data.count());
		for (int i=0; i<data.count(); ++i)
		{
			bool ok = true;
			double value = data[i].toDouble(&ok);
			if(ok)
			{
				new_data.append(value);
			}
			else
			{
				new_data.append(map[data[i]]);
			}
		}

		//replace column
		QString header = data_->column(col_index).header();
		data_->replaceColumn(col_index, header, new_data, true);
	}
}

void DataGrid::sortColumn_(bool reverse)
{
	int column = selectedColumns()[0];
	data_->column(column).sort(reverse);
}

void DataGrid::sortByColumn_(bool reverse)
{
	int column = selectedColumns()[0];
	data_->sortByColumn(column, reverse);
}

void DataGrid::sortColumnReverse_()
{
	sortColumn_(true);
}

void DataGrid::sortByColumnReverse_()
{
	sortByColumn_(true);
}

void DataGrid::addColumn_()
{
	if(data_==0 || data_->columnCount()==0)
	{
		QMessageBox::warning(this, "Error", "Can not add a column with 0 rows.");
		return;
	}

	AddColumnDialog dlg(data_->headers(), this);
	if (!dlg.exec()) return;

	//no formula - fixed text
	if (!dlg.isFormula())
	{
		QVector<QString> new_values(data_->rowCount(), dlg.value());
		data_->addColumn(dlg.name(), new_values, true, dlg.insertBefore());
		return;
	}
	//formula
	try
	{
		//make space around brackets
		QString formula = dlg.value();
		formula.replace("[", " [");
		formula.replace("]", "] ");

		//check the columns used in the formula
		QStringList parts = formula.split(QRegExp("\\s+"));
		QMap<int, QString> cols;
		for(int i=0; i<parts.size(); ++i)
		{
			QString part = parts[i];
			if (part.startsWith("[") && part.endsWith("]"))
			{
				bool ok = true;
				int index = part.mid(1, part.size()-2).toInt(&ok);

				if (!ok)
				{
					THROW(Exception,"Cannot convert '" + part + "' to a column index.");
				}
				else if (index<0 || index>=data_->columnCount())
				{
					THROW(Exception,"Column index '" + part + "' is out of bounds.");
				}
				else if (data_->column(index).type()==BaseColumn::STRING)
				{
					THROW(Exception,"Column '" + part + "' is a string column.");
				}
				else if (!cols.contains(index))
				{
					cols[index] = part;
				}
			}
		}

		//caluculate new values
		int rows_count = data_->rowCount();
		QVector<double> new_values;
		new_values.reserve(rows_count);
		QJSEngine engine;
		for (int row=0; row<rows_count; ++row)
		{
			QString row_formula = formula;

			QMapIterator<int, QString> i(cols);
			while (i.hasNext())
			{
				i.next();
				row_formula.replace(i.value(), QString::number(data_->numericColumn(i.key()).value(row), 'f', 10));
			}

			QJSValue value = engine.evaluate(row_formula);
			if (value.isError())
			{
				THROW(Exception,"The formula '" + row_formula + "' could not be evaluated.");
			}

			new_values.append(value.toNumber());
		}

		data_->addColumn(dlg.name(), new_values, true, dlg.insertBefore());
	}
	catch (Exception& e)
	{
		QMessageBox::warning(this, "Could not add a column", e.message());
	}
}

void DataGrid::copySelectionToClipboard_()
{
	QString selected_text = "";

	//column selection
	SelectionInfo info = selectionInfo();
	if (info.isColumnSelection)
	{
		QList<int> cols = selectedColumns();

		//copy header
		selected_text = "#" + data_->column(cols[0]).header();
		for (int col=1; col<cols.count(); ++col)
		{
			selected_text.append("\t" + data_->column(cols[col]).header());
		}

		//copy rows
		for (int row=0; row<rowCount(); ++row)
		{
			selected_text.append("\n");
			selected_text.append(item(row, cols[0])->text());
			for (int col=1; col<cols.count(); ++col)
			{
				selected_text.append("\t" + item(row, cols[col])->text());
			}
		}
	}
	//row selection
	else if (info.isRowSelection)
	{
		QList<int> rows = selectedRows();

		//copy header
		if (rows.count()>1)
		{
			selected_text = "#" + data_->column(0).header();
			for (int col=1; col<data_->columnCount(); ++col)
			{
				selected_text.append("\t" + data_->column(col).header());
			}
		}

		//copy rows
		for (int row=0; row<rows.count(); ++row)
		{
			if (!selected_text.isEmpty())
			{
				selected_text.append("\n");
			}
			selected_text.append(item(rows[row], 0)->text());
			for (int col=1; col<data_->columnCount(); ++col)
			{
				selected_text.append("\t" + item(rows[row], col)->text());
			}
		}
	}
	//single selection range
	else if (info.isSingleSelection)
	{
		QTableWidgetSelectionRange range = selectedRanges()[0];

		//copy header
		if (range.rowCount()>1)
		{
			selected_text = "#" + data_->column(range.leftColumn()).header();
			for (int col=range.leftColumn()+1; col<=range.rightColumn(); ++col)
			{
				selected_text.append("\t" + data_->column(col).header());
			}
		}

		//copy rows
		for (int row=range.topRow(); row<=range.bottomRow(); ++row)
		{
			if (!selected_text.isEmpty())
			{
				selected_text.append("\n");
			}
			selected_text.append(item(row, range.leftColumn())->text());
			for (int col=range.leftColumn()+1; col<=range.rightColumn(); ++col)
			{
				selected_text.append("\t" + item(row, col)->text());
			}
		}
	}

	QApplication::clipboard()->setText(selected_text);
}

void DataGrid::pasteColumn_()
{
	DataSet data_tmp;
	try
	{
		QString text = QApplication::clipboard()->text();
		QTextStream stream(&text);
		TextFile::fromStream(data_tmp, stream, "[clipboard]");
	}
	catch (FileIOException& e)
	{
		QMessageBox::warning(this, "Error pasting data.", e.message());
		return;
	}

	if (data_->rowCount()!=0 && data_tmp.rowCount()!=data_->rowCount())
	{
		QMessageBox::warning(this, "Error pasting data.", "The number of rows does not match.\nExpected " + QString::number(data_->rowCount()) + " rows, but got " + QString::number(data_tmp.rowCount()) + ".");
		return;
	}

	// Determine index
	int index = -1;
	if (selectedColumns().size()==1)
	{
		index = selectedColumns()[0];
	}

	// Insert columns
	int cols_inserted = data_tmp.columnCount();
	for (int i=0; i<cols_inserted; ++i)
	{
		BaseColumn& col = data_tmp.column(i);
		if (col.type()==BaseColumn::NUMERIC)
		{
			data_->addColumn(data_tmp.column(i).header(), data_tmp.numericColumn(i).values(), true, index);
		}
		else
		{
			data_->addColumn(data_tmp.column(i).header(), data_tmp.stringColumn(i).values(), true, index);
		}
	}
}

void DataGrid::pasteDataset_()
{
	QString text = QApplication::clipboard()->text();
	QTextStream stream(&text);
	TextImportPreview preview(stream, "[clipboard]", false, this);
	if(!preview.exec())
	{
		return;
	}

	data_->blockSignals(true);
	TextFile::fromStream(*data_, stream, "[clipboard]", preview.parameters());
	data_->blockSignals(false);

	render();
}

void DataGrid::keyPressEvent(QKeyEvent* event)
{
	if(event->matches(QKeySequence::Copy) )
	{
		copySelectionToClipboard_();
	}
	else if(event->matches(QKeySequence::Paste) )
	{
		if (columnCount()!=0)
		{
			pasteColumn_();
		}
		else
		{
			pasteDataset_();
		}
	}
	else if (event->key()==Qt::Key_F2 && event->modifiers() == Qt::NoModifier)
	{
		if (selectedItems().count()==1)
		{
			editCurrentItem(selectedItems()[0]);
		}
	}
	else
	{
		QTableWidget::keyPressEvent(event);
	}
}

void DataGrid::renderHeaders()
{
	bool show_column_index = Settings::contains("show_column_index") && Settings::boolean("show_column_index");

	for (int c=0; c<data_->columnCount(); ++c)
	{
		QString header = data_->column(c).headerOrIndex(c, show_column_index);
		QTableWidgetItem* item  = new QTableWidgetItem(header);
		if (data_->column(c).type()==BaseColumn::STRING)
		{
			QFont font;
			font.setItalic(true);
			item->setFont(font);
		}
		setHorizontalHeaderItem(c, item);
		item->setTextAlignment(Qt::AlignLeft);
	}
}

void DataGrid::editFilter_()
{
	editFilter(selectedColumns()[0]);
}

void DataGrid::removeFilter_()
{
	removeFilter(selectedColumns()[0]);
}

void DataGrid::removeDuplicates_()
{
	int col = selectedColumns()[0];

	//count values
	QHash<QString, QList<int>> value_to_rows;
	for (int r=0; r<data_->rowCount(); ++r)
	{
		QString value = data_->column(col).string(r);
		value_to_rows[value].append(r);
	}

	//remove duplicates
	QSet<int> rows_to_keep;
	for (auto it=value_to_rows.begin(); it!=value_to_rows.end(); ++it)
	{
		rows_to_keep << it.value()[0];
	}

	//reduce to rows without duplicates
	data_->reduceToRows(rows_to_keep);
}

void DataGrid::keepDuplicates_()
{
	int col = selectedColumns()[0];

	//count values
	QHash<QString, QSet<int>> value_to_rows;
	for (int r=0; r<data_->rowCount(); ++r)
	{
		QString value = data_->column(col).string(r);
		value_to_rows[value].insert(r);
	}

	//determine duplicates
	QSet<int> rows_to_keep;
	for (auto it=value_to_rows.begin(); it!=value_to_rows.end(); ++it)
	{
		if (it.value().count()>1)
		{
			rows_to_keep.unite(it.value());
		}
	}

	//reduce to rows with duplicates
	data_->reduceToRows(rows_to_keep);
}

void DataGrid::editFilter(int column)
{
	FilterDialog* dialog = new FilterDialog(&data_->column(column), this);
	if (dialog->exec())
	{
		render();
	}
}

void DataGrid::removeFilter(int column)
{
	data_->column(column).setFilter(Filter());

	render();
}

void DataGrid::removeAllFilters()
{
	for (int c=0; c<data_->columnCount(); ++c)
	{
		data_->column(c).setFilter(Filter());
	}

	render();
}

void DataGrid::reduceToFiltered()
{
	QBitArray filtered_rows = data_->getRowFilter(false);
	if (filtered_rows.count(true)==data_->rowCount())
	{
		return;
	}

	data_->blockSignals(true);

	for (int c=0; c<data_->columnCount(); ++c)
	{
		//numeric column
		if (data_->column(c).type()==BaseColumn::NUMERIC)
		{
			NumericColumn& column = data_->numericColumn(c);

			QVector<double> values;
			values.reserve(filtered_rows.count());
			for (int r=0; r<filtered_rows.count(); ++r)
			{
				if (filtered_rows[r])
				{
					values.append(column.value(r));
				}
			}

			column.setValues(values);
		}
		//string column
		else
		{
			StringColumn& column = data_->stringColumn(c);

			QVector<QString> values;
			values.reserve(filtered_rows.count());
			for (int r=0; r<filtered_rows.count(); ++r)
			{
				if (filtered_rows[r])
				{
					values.append(column.value(r));
				}
			}

			column.setValues(values);
		}

		//remove filter
		data_->column(c).setFilter(Filter());
	}

	data_->blockSignals(false);

	// trigger filter widget update by setting the same value for "filters enabled"
	data_->setFiltersEnabled(data_->filtersEnabled());

	// trigger rendering (signal have been blocked)
	render();
}

QVector< QPair<int, int> > DataGrid::findItems(QString text, Qt::CaseSensitivity case_sensitive, DataGrid::FindType type) const
{
	QVector< QPair<int, int> > hits;

	if (type == DataGrid::FIND_EXACT)
	{
		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (item(row, col)->text().compare(text, case_sensitive) == 0)
				{
					hits.append(qMakePair(col, row));
				}
			}
		}
	}
	else if (type == DataGrid::FIND_CONTAINS)
	{
		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (item(row, col)->text().contains(text, case_sensitive))
				{
					hits.append(qMakePair(col, row));
				}
			}
		}
	}
	else if (type == DataGrid::FIND_START)
	{
		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (item(row, col)->text().startsWith(text, case_sensitive))
				{
					hits.append(qMakePair(col, row));
				}
			}
		}
	}
	else if (type == DataGrid::FIND_END)
	{
		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (item(row, col)->text().endsWith(text, case_sensitive))
				{
					hits.append(qMakePair(col, row));
				}
			}
		}
	}
	else if (type == DataGrid::FIND_REGEXP)
	{
		QRegExp regexp(text, case_sensitive, QRegExp::RegExp2);

		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (regexp.indexIn(item(row, col)->text()) != -1)
				{
					hits.append(qMakePair(col, row));
				}
			}
		}
	}

	return hits;
}

QString DataGrid::findTypeToString(DataGrid::FindType type)
{
	switch(type)
	{
		case DataGrid::FIND_EXACT:
			return "is";
		case DataGrid::FIND_CONTAINS:
			return "contains";
		case DataGrid::FIND_START:
			return "starts with";
		case DataGrid::FIND_END:
			return "ends with";
		case DataGrid::FIND_REGEXP:
			return "matches (QRegExp)";
	}

	THROW(FindTypeException,"Unknown find type!");
}

void DataGrid::columnChanged(int column, bool until_end)
{
	if (data_->column(column).filter().type()!=Filter::NONE)
	{
		render();
	}
	else
	{
		int end = column + 1;
		if (until_end)
		{
			end = columnCount();
		}

		for (int c=column; c<end; ++c)
		{
			renderColumn_(c, data_->getRowFilter(false));
		}
	}
}

void DataGrid::editCurrentItem(QTableWidgetItem* item)
{
	if (preview_>0 || item==0)
	{
		return;
	}

	int col = item->column();
	int row = item->row();

	//correct row if filtered
	QBitArray filter = data_->getRowFilter();
	if (filter.count(true)!=data_->rowCount())
	{
		int r_filtered = -1;
		for (int i=0; i<data_->rowCount(); ++i)
		{
			if (filter[i])
			{
				++r_filtered;
			}

			if (r_filtered==row)
			{
				row = i;

				break;
			}
		}
	}

	//edit numeric columns
	if (data_->column(col).type() == BaseColumn::NUMERIC)
	{
		double value = data_->numericColumn(col).value(row);
		double new_value = QInputDialog::getDouble(this, "Edit numeric item", "Value", value, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 5);
		if (new_value != value)
		{
			data_->numericColumn(col).setValue(row, new_value);
		}
	}
	//edit string column
	else
	{
		TextItemEditDialog dlg(this);
		dlg.setText(data_->stringColumn(col).value(row));
		if (dlg.exec()==QDialog::Accepted)
		{
			data_->stringColumn(col).setValue(row, dlg.text());
		}
	}
}

bool DataGrid::isNumeric_(QString string)
{
	bool ok = true;
	string.toDouble(&ok);
	return ok;
}

void DataGrid::loadFilter()
{
	//get list of available filters
	QMap<QString, QVariant> filters = Settings::map("filters", true);

	//abort if there are no filters
	if (filters.count()==0)
	{
		QMessageBox::information(this, "Load filter set", "There are no stored filter sets!");
		return;
	}

	//select filter
	bool ok = true;
	QString name = QInputDialog::getItem(this, "Load filter set", "Filter:", filters.keys(), 0, false, &ok);
	if (!ok) return;

	//apply filter
	filtersFromString(filters[name].toString());
}

void DataGrid::storeFilter()
{
	//check that there is a filter to store
	QString filter_string = filtersAsString();
	if (filter_string=="")
	{
		QMessageBox::information(this, "Store filter set", "There are no filter rules to store!");
		return;
	}

	//get new filter name
	QString name = QInputDialog::getText(this, "Enter filter name", "Filter:");
	if (name=="") return;

	//get list of available filters
	QMap<QString, QVariant> filters = Settings::map("filters", true);

	//check before overwriting
	if (filters.contains(name))
	{
		int result = QMessageBox::question(this, "Store filter set", "A filter with the name '" + name + "' already exists. Do you want to overwrite it?");
		if (result==QMessageBox::No) return;
	}

	//store filters
	filters[name] = filter_string;
	Settings::setMap("filters", filters);
}

void DataGrid::deleteFilter()
{
	//get list of available filters
	QMap<QString, QVariant> filters = Settings::map("filters", true);
	if (filters.count()==0)
	{
		QMessageBox::information(this, "Delete filter set", "There are no filter sets to delete!");
		return;
	}

	//show dialog
	QComboBox* box = new QComboBox();
	box->addItems(filters.keys());
	auto dlg = GUIHelper::createDialog(box, "Delete filter set", "", true);
	if (dlg->exec()!=QDialog::Accepted) return;

	//remove filter set
	QString name = box->currentText();
	filters.remove(name);
	Settings::setMap("filters", filters);
}


QString DataGrid::filtersAsString()
{
	QString output = "";
	for (int i=0; i<data_->columnCount(); ++i)
	{
		BaseColumn& col = data_->column(i);
		if (col.filter().type()==Filter::NONE) continue;

		if (output!="") output += ";";
		output += col.header() + ":" + Filter::typeToString(col.filter().type(), false) + ":" + col.filter().value();

	}
	return output;
}


void DataGrid::filtersFromString(QString filter_string)
{
	//check that filter columns are present
	QStringList filters = filter_string.split(";");
	for (int i=0; i<filters.count(); ++i)
	{
		QStringList parts = filters[i].split(":");
		int index = data_->indexOf(parts[0]);
		if (index==-1)
		{

			QMessageBox::information(this, "Load filter error", "The filter column '" + parts[0] + "' is not present!");
			return;
		}
	}

	//clear current filters
	data_->blockSignals(true);
	for (int i=0; i<filters.count(); ++i)
	{
		data_->column(i).setFilter(Filter());
	}

	//apply filters
	for (int i=0; i<filters.count(); ++i)
	{
		QStringList parts = filters[i].split(":");

		Filter filter;
		filter.setType(Filter::stringToType(parts[1], false));
		filter.setValue(parts[2]);

		int index = data_->indexOf(parts[0]);
		try
		{
			data_->column(index).setFilter(filter);
		}
		catch(FilterTypeException& e)
		{
			QMessageBox::warning(this, "Load filter set", "Incompatible filter '" + parts[1] + ":" + parts[2] +"' for column '" + data_->column(index).headerOrIndex(index) + "'.\nError message: " + e.message());

			//clear filter loaded so far
			for (int i=0; i<filters.count(); ++i)
			{
				data_->column(i).setFilter(Filter());
			}

			//abort applying filters
			break;
		}
	}
	data_->blockSignals(false);

	// trigger rendering (signal have been blocked)
	data_->setFiltersEnabled(true);
	render();
}

