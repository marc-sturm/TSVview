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
#include <QRegularExpression>
#include <algorithm>
#include <math.h>
#include <QTemporaryFile>
#include "GrepDialog.h"
#include "DataGrid.h"
#include "CustomExceptions.h"
#include "Settings.h"
#include "TextImportPreview.h"
#include "FilterDialog.h"
#include "ReplacementDialog.h"
#include "MergeDialog.h"
#include "GUIHelper.h"
#include "AddColumnDialog.h"
#include "TextItemEditDialog.h"
#include "Helper.h"

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
    setStyleSheet("QTableWidget:!active { selection-color: "+fg+"; selection-background-color: "+bg+"; }");

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
	output.isSingleRangeSelection = ranges.count() == 1;

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

void DataGrid::render()
{
    //store colum widths
    QHash<QString, int> widths_before;
    for (int c=0; c<columnCount(); ++c)
    {
        QString name = horizontalHeaderItem(c)->text();
        if (name.startsWith("[") && name.contains("] ")) name = name.mid(name.indexOf(" ")+1);
        widths_before[name] = columnWidth(c);
    }

    //clear
    setColumnCount(0);
    setRowCount(0);

	//abort if dataset is not set
	if (data_==0)
	{
		clear();
		return;
	}

	QElapsedTimer timer;
	timer.start();

    // get row/column count

	//determine number of rows to render (if we need to filter)
	QBitArray rows_to_render = data_->getRowFilter();
	int rows = rows_to_render.count(true);

	//set table to new dimensions
	if (preview_>0)
	{
		rows = std::min(rows, preview_);
	}

	//render
    int cols = data_->columnCount();
	setColumnCount(cols);
	setRowCount(rows);
	renderHeaders();
	for (int c=0; c<cols; ++c)
	{
		renderColumn_(c, rows_to_render);
	}

    //restore column width
    for (int c=0; c<columnCount(); ++c)
    {
        QString name = data_->column(c).header();
        if (widths_before.contains(name))
        {
            setColumnWidth(c, widths_before[name]);
        }
    }

	//add preview signs (if required)
	if (preview_>0 && rows==preview_)
	{
		for (int c=0; c<cols; ++c)
		{
			setItem(preview_-1, c, new QTableWidgetItem("..."));
		}
	}

	qDebug() << "rendering table: c=" << cols << "r=" << rows << "ms=" << timer.restart();

	emit rendered();
}

QMenu* DataGrid::createStandardContextMenu()
{
	QAction* action = 0;
	QMenu* menu = new QMenu();

	action = menu->addAction(QIcon(":/Icons/Copy.png"), "Copy", this, SLOT(copySelectionToClipboard_()));
	action->setEnabled(!selectedRanges().isEmpty());
	action = menu->addAction(QIcon(":/Icons/Copy.png"), "Copy - German numbers", this, SLOT(copySelectionToClipboardGerman_()));
	action->setEnabled(!selectedRanges().isEmpty());

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
        action = edit_menu->addAction("Set decimals", this, SLOT(setDecimals_()));
        action->setEnabled(selected_count>0 && text_count==0);
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

	data_->removeColumns(LIST_TO_SET(selectedColumns()));
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

void DataGrid::convertNumericNan_()
{
	//convert
	int col_index = selectedColumns().at(0);
    const QVector<QString>& data = data_->stringColumn(col_index).values();
	QVector<double> new_data;
    new_data.reserve(data.count());
    QVector<char> new_decimals;
    new_decimals.reserve(data.count());
	for (int i=0; i<data.count(); ++i)
	{
        auto tmp = NumericColumn::toDouble(data[i], true);
        new_data << tmp.first;
        new_decimals << tmp.second;
	}

	//replace column
	QString header = data_->column(col_index).header();
    data_->replaceColumn(col_index, header, new_data, new_decimals);
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
	int col_index = selectedColumns().at(0);
	QVector<QString> data = data_->stringColumn(col_index).values();
	QVector<double> new_data;
    new_data.reserve(data.count());
    QVector<char> new_decimals;
    new_decimals.reserve(data.count());
	for (int i=0; i<data.count(); ++i)
	{
        auto tmp = NumericColumn::toDouble(data[i], true);
        if (std::isnan(tmp.first)) tmp.first = fallback_value;
        new_data << tmp.first;
        new_decimals << tmp.second;
	}

	//replace column
	QString header = data_->column(col_index).header();
    data_->replaceColumn(col_index, header, new_data, new_decimals);
}


void DataGrid::convertNumericDict_()
{
	int col_index = selectedColumns().at(0);

	//create list of not-convertable values
	int max_count = 20;
	QSet<QString> not_convertable;
	QVector<QString> data = data_->stringColumn(col_index).values();
	for (int i=0; i<data.count(); ++i)
	{
        if (!Helper::isNumeric(data[i]))
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
		QMap<QString, QPair<double, char>> map = dialog->getMap();

		//create new column
		QVector<double> new_data;
		new_data.reserve(data.count());
        QVector<char> new_decimals;
        new_decimals.reserve(data.count());
		for (int i=0; i<data.count(); ++i)
        {
            auto tmp = NumericColumn::toDouble(data[i], true);
			if (std::isnan(tmp.first)) tmp = map[data[i]];
            new_data << tmp.first;
            new_decimals << tmp.second;
		}

		//replace column
		QString header = data_->column(col_index).header();
        data_->replaceColumn(col_index, header, new_data, new_decimals);
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
        data_->addColumn(dlg.name(), new_values, dlg.insertBefore());
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
		QStringList parts = formula.split(QRegularExpression("\\s+"));
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

            new_values << value.toNumber();
		}

        data_->addColumn(dlg.name(), new_values, QVector<char>(rows_count, dlg.decimals()), dlg.insertBefore());
	}
	catch (Exception& e)
	{
        QMessageBox::warning(this, "Error while adding numeric column", e.message());
	}
}

void DataGrid::copySelectionToClipboard_()
{
	copySelectionToClipboard_('.');
}

void DataGrid::copySelectionToClipboardGerman_()
{
	copySelectionToClipboard_(',');
}

void DataGrid::copySelectionToClipboard_(QChar decimal_point)
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
			for (int col=0; col<cols.count(); ++col)
			{
				if (col!=0) selected_text.append("\t");
				selected_text.append(itemText(row, cols[col], data_->column(cols[col]).type()==BaseColumn::NUMERIC, decimal_point));
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
			for (int col=0; col<data_->columnCount(); ++col)
			{
				if (col!=0) selected_text.append("\t");
				selected_text.append(itemText(rows[row], col, data_->column(col).type()==BaseColumn::NUMERIC, decimal_point));
			}
		}
	}
	//single selection range
	else if (info.isSingleRangeSelection)
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
			for (int col=range.leftColumn(); col<=range.rightColumn(); ++col)
			{
				if (col!=range.leftColumn()) selected_text.append("\t");
				selected_text.append(itemText(row, col, data_->column(col).type()==BaseColumn::NUMERIC, decimal_point));
			}
		}
	}

	QApplication::clipboard()->setText(selected_text);
}

void DataGrid::pasteColumn_(int index)
{
	//load data from clipboard into tmp dataset
	DataSet data_tmp;
	try
	{        
        //create tmp file (with automatic cleanup)
        QString tmp_filename = Helper::tempFileName(".txt");
        auto cleanup = qScopeGuard([&] { QFile::remove(tmp_filename); });

        //store clipboard to tmp file
        Helper::storeTextFile(tmp_filename, QApplication::clipboard()->text().split('\n'));

        //load data
        data_tmp.load(tmp_filename, "[clipboard]");
	}
    catch (FileAccessException& e)
	{
		QMessageBox::warning(this, "Error parsing data from clipboard!", e.message());
		return;
	}

	//check if entire column is pasted
	if (data_->rowCount()!=0 && data_tmp.rowCount()!=data_->rowCount())
	{
		QMessageBox::warning(this, "Error pasting data.", "The number of rows does not match.\nExpected " + QString::number(data_->rowCount()) + " rows, but got " + QString::number(data_tmp.rowCount()) + ".");
		return;
	}

	//insert columns
    data_->blockSignals(true);
	for (int i=0; i<data_tmp.columnCount(); ++i)
	{
		BaseColumn& col = data_tmp.column(i);
		if (col.type()==BaseColumn::NUMERIC)
		{
            data_->addColumn(data_tmp.column(i).header(), data_tmp.numericColumn(i).values(), data_tmp.numericColumn(i).decimals(), index);
		}
		else
		{
            data_->addColumn(data_tmp.column(i).header(), data_tmp.stringColumn(i).values(), index);
		}
	}
    data_->blockSignals(false);

    //re-render
    render();
}

void DataGrid::pasteDataset_()
{
    //create tmp file (with automatic cleanup)
    QString tmp_filename = Helper::tempFileName(".txt");
    auto cleanup = qScopeGuard([&] { QFile::remove(tmp_filename); });

    //store clipboard to tmp file
    Helper::storeTextFile(tmp_filename, QApplication::clipboard()->text().split('\n'));

    //show import dialog
    TextImportPreview preview(tmp_filename, "[clipboard]", false, this);
    if(!preview.exec()) return;

	data_->blockSignals(true);
    data_->import(tmp_filename, "[clipboard]", preview.parameters());
    data_->blockSignals(false);
    data_->setModified(true, true);
	render();
}

void DataGrid::keyPressEvent(QKeyEvent* event)
{
	bool handled = false;
	if (event->key()==Qt::Key_C && (QGuiApplication::keyboardModifiers()&Qt::ControlModifier) && (QGuiApplication::keyboardModifiers()&Qt::ShiftModifier))
	{
		copySelectionToClipboardGerman_();
		handled = true;
	}
	else if(event->matches(QKeySequence::Copy))
	{
		copySelectionToClipboard_();
		handled = true;
	}
	else if(event->matches(QKeySequence::Paste))
	{
        QList<int> selected_cols = selectedColumns();
        QList<QTableWidgetItem*> selected_items = selectedItems();
        if (columnCount()==0)
		{
			pasteDataset_();
		}
        else if (selected_cols.count()==1 && selected_items.count()==rowCount())
		{
            pasteColumn_(selected_cols[0]);
		}
        else if (selected_cols.count()==0 && selected_items.count()==0)
        {
            pasteColumn_(-1);
        }
        else if (selected_items.count()==1)
		{
			QString text = QApplication::clipboard()->text();
			if (!text.contains("\t"))
			{
				while (text.endsWith('\n') || text.endsWith('\r')) text.chop(1);
				if (!text.contains("\n"))
				{
					try
					{
                        int col = selected_items[0]->column();
                        int row = correctRowIfFiltered(selected_items[0]->row());
						data_->column(col).setString(row, text);
					}
					catch (Exception& e)
					{
						QMessageBox::warning(this, "Error pasting single value!", e.message());
						return;
					}
				}
			}
		}

		handled = true;
	}
	else if (event->key()==Qt::Key_F2 && event->modifiers() == Qt::NoModifier)
	{
		if (selectedItems().count()==1)
		{
			editCurrentItem(selectedItems()[0]);
		}
		handled = true;
	}
	else if(event->matches(QKeySequence::Delete))
	{
		if (selectedItems().count()==1)
		{
			int col = selectedItems()[0]->column();
			int row = correctRowIfFiltered(selectedItems()[0]->row());
			data_->column(col).setString(row, "");

			handled = true;
		}
	}

	if (!handled) QTableWidget::keyPressEvent(event);
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
    if (selectedColumns().count()==0) return;

	int col = selectedColumns()[0];

	//count values
    QHash<QString, int> value_to_first_row;
	for (int r=0; r<data_->rowCount(); ++r)
	{
		QString value = data_->column(col).string(r);
        if (!value_to_first_row.contains(value))
        {
            value_to_first_row[value] = r;
        }
	}

	//remove duplicates
	QSet<int> rows_to_keep;
    for (auto it=value_to_first_row.begin(); it!=value_to_first_row.end(); ++it)
	{
        rows_to_keep << it.value();
	}

	//reduce to rows without duplicates
	data_->reduceToRows(rows_to_keep);
}

void DataGrid::keepDuplicates_()
{
    if (selectedColumns().count()==0) return;

	int col = selectedColumns()[0];

	//count values
    QHash<QString, QSet<int>> value_to_rows; //TODO just count (int instead of QSet<int>)
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

void DataGrid::grepLines()
{
	GrepDialog dlg(this);
	if (dlg.exec()!=QDialog::Accepted) return;

	QString operation = dlg.operation();
	QString value = dlg.value();
	Qt::CaseSensitivity case_sensitive = dlg.caseSensitivity();

	//determine matches
	QBitArray matches(data_->rowCount(), false);
	for (int r=0; r<data_->rowCount(); ++r)
	{
		for (int c=0; c<data_->columnCount(); ++c)
		{
			if(data_->column(c).string(r).contains(value, case_sensitive))
			{
				matches[r] = true;
				break;
			}
		}
	}

	//invert if requested
	if (operation!="contains") matches = ~matches;

	//apply
	QSet<int> rows_to_keep;
	for (int r=0; r<matches.count(); ++r)
	{
        if (matches[r]) rows_to_keep << r;
	}

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
            QVector<char> decimals;
            decimals.reserve(filtered_rows.count());
			for (int r=0; r<filtered_rows.count(); ++r)
			{
				if (filtered_rows[r])
				{
					values.append(column.value(r));
                    decimals.append(column.decimals(r));
				}
			}

            column.setValues(values, decimals);
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
		QRegularExpression regexp(text, case_sensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);

		for (int row=0; row<rowCount(); ++row)
		{
			for (int col=0; col<columnCount(); ++col)
			{
				if (regexp.match(item(row, col)->text()).hasMatch())
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
            return "matches (regular expression))";
	}

    THROW(ProgrammingException, "Unknown find type integer '"+QString::number(type)+"'!");
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
	int row = correctRowIfFiltered(item->row());

	//edit numeric columns
	if (data_->column(col).type() == BaseColumn::NUMERIC)
	{
        NumericColumn& column = data_->numericColumn(col);
        double value = column.value(row);
		double new_value = QInputDialog::getDouble(this, "Edit numeric item", "Value", value, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 5);
		if (new_value != value)
		{
            column.setValue(row, new_value);
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

void DataGrid::setDecimals_()
{
    bool ok = true;
    int decimals = QInputDialog::getInt(this, "Set decimals for numeric columns", "decimals:", 2, 0, 999, 1, &ok);
    if (!ok) return;

    foreach (int c, selectedColumns())
    {
        NumericColumn& col = data_->numericColumn(c);
        col.setDecimals(QVector<char>(col.count(), decimals));
    }
}

QString DataGrid::itemText(int row, int col, bool is_numeric, QChar decimal_point)
{
	QString text = item(row, col)->text();

	if(is_numeric && decimal_point!='.')
	{
		text.replace('.', ',');
	}

	return text;
}

int DataGrid::correctRowIfFiltered(int row) const
{
	if (data_->filtersEnabled())
	{
		QBitArray filter = data_->getRowFilter();

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

	return row;
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

void DataGrid::resizeColumnWidth()
{
    QElapsedTimer timer;
    timer.start();

	GUIHelper::resizeTableCellWidths(this, -1, 1000);

    //limit column width to 50% of window width
	int max_width = 0.9 * width();
    for (int i=0; i<columnCount(); ++i)
    {
        if (columnWidth(i)>max_width)
        {
            setColumnWidth(i, max_width);
        }
    }

    qDebug() << "resizing column width to content: ms=" << timer.elapsed();
}

void DataGrid::resizeColumnHeight()
{
    QElapsedTimer timer;
    timer.start();

    GUIHelper::resizeTableCellHeightsToMinimum(this, 1000);

    qDebug() << "resizing column height to content: ms=" << timer.elapsed();
}

QList<int> DataGrid::columnWidths() const
{
    QList<int> output;
    for (int c=0; c<columnCount(); ++c)
    {
       output << columnWidth(c);
    }
    return output;
}

