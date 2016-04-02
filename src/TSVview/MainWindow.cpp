#include <QFileDialog>
#include <QClipboard>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QWindowList>
#include <QMimeData>
#include <QWindow>
#include <QTextBrowser>

#include "MainWindow.h"
#include "TextFile.h"
#include "TextImportPreview.h"
#include "XMLFile.h"
#include "XMLImportPreview.h"
#include "HistogramPlot.h"
#include "StatisticsSummaryWidget.h"
#include "CustomExceptions.h"
#include "ScatterPlot.h"
#include "DataPlot.h"
#include "BoxPlot.h"
#include "Settings.h"
#include "Smoothing.h"
#include "ZXVFile.h"
#include "Filter.h"
#include "CustomExceptions.h"
#include "GUIHelper.h"
#include "ScrollableTextDialog.h"
#include "Helper.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, data_()
	, recent_files_()
{
	ui_.setupUi(this);

	//create info widget in status bar
	info_widget_ = new QLabel("cols: 0 rows: 0");
	statusBar()->addPermanentWidget(info_widget_);

	//create grid and dataset
	grid_ = new DataGrid();
	setCentralWidget(grid_);
	connect(grid_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenu(QPoint)));
	connect(grid_, SIGNAL(rendered()), this, SLOT(updateInfoWidget()));
	connect(&data_, SIGNAL(modificationStatusChanged(bool)), this, SLOT(datasetStatusChanged(bool)));
	connect(&data_, SIGNAL(filtersChanged()), this, SLOT(updateFilters()));
	connect(&data_, SIGNAL(headersChanged()), this, SLOT(updateFilters()));
	grid_->setData(data_);

	recent_files_ = Settings::stringList("recent_files");
	updateRecentFilesMenu_();

	//create goto dock widget
	goto_widget_ = new GoToDockWidget();
	goto_widget_->setVisible(false);
	addDockWidget(Qt::BottomDockWidgetArea, goto_widget_);
	connect(goto_widget_, SIGNAL(goToLine(int)), this, SLOT(goToRow(int)));

	//create find dock widget
	find_widget_ = new FindDockWidget();
	find_widget_->setVisible(false);
	addDockWidget(Qt::BottomDockWidgetArea, find_widget_);
	connect(find_widget_, SIGNAL(searchForText(QString, Qt::CaseSensitivity, DataGrid::FindType)), this, SLOT(findText(QString, Qt::CaseSensitivity, DataGrid::FindType)));
	connect(find_widget_, SIGNAL(searchNext()), this, SLOT(findNext()));

	//create filter widget
	filter_widget_ = new FilterWidget();
	filter_widget_->setVisible(false);
	addDockWidget(Qt::RightDockWidgetArea, filter_widget_);
	connect(filter_widget_, SIGNAL(filterEnabledChanged(bool)), this, SLOT(toggleFilter(bool)));
	connect(filter_widget_, SIGNAL(editFilter(int)), grid_, SLOT(editFilter(int)));
	connect(filter_widget_, SIGNAL(removeFilter(int)), grid_, SLOT(removeFilter(int)));
	connect(filter_widget_, SIGNAL(removeAllFilters()), grid_, SLOT(removeAllFilters()));
	connect(filter_widget_, SIGNAL(reduceToFiltered()), grid_, SLOT(reduceToFiltered()));
	connect(filter_widget_, SIGNAL(loadFilter()), grid_, SLOT(loadFilter()));
	connect(filter_widget_, SIGNAL(storeFilter()), grid_, SLOT(storeFilter()));
	connect(filter_widget_, SIGNAL(deleteFilter()), grid_, SLOT(deleteFilter()));

	//enable drop event
	setAcceptDrops(true);

	//init file
	setFile("untitled");
	connect(&file_watcher_, SIGNAL(fileChanged()), this, SLOT(fileChanged()));

	//load argument file
	if (QApplication::arguments().count()==2)
	{
		QString filename = QApplication::arguments()[1];
		openFile_(filename, getType(filename), true);
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	storeModifiedDataset_();

	Settings::setStringList("recent_files", recent_files_);

	event->accept();
}

void MainWindow::storeModifiedDataset_()
{
	if (data_.modified() && (file_.type!=NONE || data_.columnCount()!=0))
	{
		QMessageBox box(this);
		box.setWindowTitle(QApplication::applicationName());
		box.setText("Save changes to '" + file_.name + "'?");
		box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		box.setDefaultButton(QMessageBox::No);
		if (box.exec() == QMessageBox::Yes)
		{
			on_saveFile_triggered(true);
		}
	}
}

void MainWindow::datasetStatusChanged(bool /*modified*/)
{
	updateWindowTitle_();
}

void MainWindow::updateWindowTitle_()
{
	QString title = QApplication::applicationName() + " - " + file_.name;
	if (data_.modified())
	{
		title += "*";
	}
	setWindowTitle(title);
}

void MainWindow::on_newFile_triggered(bool)
{
	storeModifiedDataset_();

	setFile("untitled");
	data_.clear(true);
}

void MainWindow::on_openZXV_triggered(bool)
{
	storeModifiedDataset_();

	QString filename = QFileDialog::getOpenFileName(this, "Open ZXV file", Settings::path("path_open"), "Zipped XML value files (*.zxv);;All files (*.*)");
	if (filename==QString::null)
	{
		return;
	}

	openFile_(filename, ZXV);
}

void MainWindow::on_openTXT_triggered(bool)
{
	storeModifiedDataset_();

	QString filename = QFileDialog::getOpenFileName(this, "Open text file", Settings::path("path_open"), "All files (*.*);;Text files (*.txt);;CSV files (*.csv);;TSV files (*.tsv)");
	if (filename==QString::null)
	{
		return;
	}

	openFile_(filename, TXT);
}

void MainWindow::on_openXML_triggered(bool)
{
	storeModifiedDataset_();

	QString filename = QFileDialog::getOpenFileName(this, "Open XML file", Settings::path("path_open"), "All files (*.*);;XML files (*.xml)");
	if (filename==QString::null)
	{
		return;
	}

	openFile_(filename, XML);
}

void MainWindow::openFile_(QString filename, FileType type, bool remember_path)
{
	//close plots
	QWindowList windows = QApplication::allWindows();
	foreach (QWindow* window, windows)
	{
		if (window->objectName()!="MainWindowWindow")
		{
			window->close();
		}
	}

	//update settings
	if (remember_path)
	{
		Settings::setPath("path_open", filename);
	}

	//reset GUI
	setFile("untitled");

	//load file
	data_.blockSignals(true);
	try
	{
		Parameters param;
		if (type==ZXV)
		{
			ZXVFile::load(data_, filename);
		}
		else if (type==TXT)
		{
			param = TextFile::defaultParameters();
			if (!filename.endsWith(".tsv"))
			{
				QFile file(filename);
				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				{
					QMessageBox::warning(this, "Error", "Could not open file '" + filename + "' for reading.");
					throw CleanUpException();
				}

				QTextStream stream(&file);
				TextImportPreview preview(stream, filename, filename.endsWith(".csv") ,this);
				if(!preview.exec()) throw CleanUpException();

				param = preview.parameters();
			}

			TextFile::load(data_, filename, param);
		}
		else if (type==XML)
		{
			XMLImportPreview preview(filename, this);
			if(!preview.exec()) throw CleanUpException();

			param =  preview.parameters();
			XMLFile::load(data_, filename, param);
		}

		setFile(filename, type, param);
	}
	catch (CleanUpException&)
	{
		//no error message, just clean up
	}
	catch (Exception& e)
	{
		QMessageBox::warning(this, "Error loading file.", e.message());
	}

	//update GUI
	updateFilters();
	grid_->render();

	//re-enable data signals
	data_.blockSignals(false);
}

void MainWindow::on_saveFile_triggered(bool)
{
	if (file_.type==NONE)
	{
		on_saveFileAs_triggered(true);
		return;
	}

	//disable file watcher
	file_watcher_.clearFile();

	try
	{
		if (file_.type == ZXV)
		{
			ZXVFile::store(data_, file_.name);
		}
		if (file_.type == TXT)
		{
			TextFile::store(data_, file_.name, file_.param);
		}
		if (file_.type == XML)
		{
			XMLFile::store(data_, file_.name, file_.param);
		}
	}
	catch (FileIOException& e)
	{
		QMessageBox::warning(this, "Error storing file '"+file_.name + "'", e.message());
	}

	//re-enable file watcher
	file_watcher_.setFile(file_.name);

	data_.setModified(false);
}

void MainWindow::on_saveFileAs_triggered(bool)
{
	QString selected_filter = "";
	QString filename = QFileDialog::getSaveFileName(this, "Save file", Settings::path("path_open") + file_.name, "Text files (*.txt *.csv *.tsv);;XML files (*.xml);;Zipped XML value files (*.zxv)", &selected_filter);
	if (filename==QString::null)
	{
		return;
	}

	Settings::setPath("path_open", filename);

	if (selected_filter=="Text files (*.txt *.csv *.tsv)")
	{
		// get parameters
		Parameters params = TextFile::defaultParameters();
		if (!ParameterEditor::asDialog("Text file parameters", params))
		{
			return;
		}

		setFile(filename, TXT, params);
	}
	else if (selected_filter=="XML files (*.xml)")
	{
		// get parameters
		Parameters params = XMLFile::defaultParameters();
		if (!ParameterEditor::asDialog("XML file parameters", params))
		{
			return;
		}

		setFile(filename, XML, params);
	}
	else if (selected_filter=="Zipped XML value files (*.zxv)")
	{
		setFile(filename, ZXV);
	}

	on_saveFile_triggered(true);
}

/*
void MainWindow::appendColumns_(const DataSet& dataset)
{
  int current_rows = data_.rowCount();
  if (current_rows>0 && dataset.rowCount()!=current_rows)
  {
	QMessageBox::warning(this, "Error", "Row count does not match! Append columns to empty datasets or datasets with the same number of rows.");
	return;
  }

  int columns_before = data_.columnCount();
  if (data_.rowCount()==0)
  {
	data_ = dataset;
  }
  else
  {
	for (int i=0; i<dataset.columnCount(); ++i)
	{
			if (dataset.column(i).type()==BaseColumn::NUMERIC)
			{
		data_.addColumn(dataset.column(i).header(), dataset.numericColumn(i).values());
			}
			else
			{
		data_.addColumn(dataset.column(i).header(), dataset.stringColumn(i).values());
			}
	}
  }

	grid_->render(columns_before);
}
*/

void  MainWindow::on_resizeColumns_triggered(bool)
{
	grid_->resizeColumnsToContents();

	//limit column width to 80% of window width
	int max_width = 0.8 * width();
	for (int i=0; i<grid_->columnCount(); ++i)
	{
		if (grid_->columnWidth(i)>max_width)
		{
			grid_->setColumnWidth(i, max_width);
		}
	}
}

void  MainWindow::on_resizeRows_triggered(bool)
{
	grid_->resizeRowsToContents();
}

void  MainWindow::on_goToRow_triggered(bool)
{
	if(!goto_widget_->isHidden())
	{
		goto_widget_->hide();
		return;
	}

	find_widget_->hide();

	goto_widget_->show();
	goto_widget_->setFocus();
}

void  MainWindow::on_findText_triggered(bool)
{
	if(!find_widget_->isHidden())
	{
		find_widget_->hide();
		return;
	}

	goto_widget_->hide();

	find_widget_->show();
	find_widget_->setFocus();
}

void  MainWindow::on_filter_triggered(bool)
{
	if (!filter_widget_->isHidden())
	{
		filter_widget_->hide();
		return;
	}

	filter_widget_->show();
}

void MainWindow::on_exit_triggered(bool)
{
	this->close();
}

void MainWindow::on_clearSettings_triggered(bool)
{
	Settings::clear();
	recent_files_.clear();

	updateRecentFilesMenu_();
}

void MainWindow::on_addToContext_triggered(bool)
{
	QString app_name = QApplication::applicationFilePath();

	QString filename = app_name +  + ".reg";

	QFile file(filename);
	if (file.open(QFile::WriteOnly | QFile::Text))
	{
		app_name.replace("/", "\\\\");

		QTextStream stream(&file);
		stream << "Windows Registry Editor Version 5.00\n";
		stream << "\n";
		stream << "[HKEY_CLASSES_ROOT\\*\\shell\\Open with TSVview]\n";
		stream << "\n";
		stream << "[HKEY_CLASSES_ROOT\\*\\shell\\Open with TSVview\\command]\n";
		stream << "@=\"" + app_name + " \\\"%1\\\"\"\n";
		stream << "\n";
		stream << "\n";
	}
	else
	{
		QMessageBox::warning(this, "Error creating file.", "The file '" + filename + "' could not be created!");
	}
	file.close();

	QDesktopServices::openUrl("file:///" + filename);
}

void MainWindow::on_transpose_triggered(bool /*checked*/)
{
	//nothing to do
	if (data_.columnCount()==0) return;

	//check that all columns are numeric
	for (int i=1; i<data_.columnCount(); ++i)
	{
		if (data_.column(i).type()!=BaseColumn::NUMERIC)
		{
			QMessageBox::warning(this, "Transpose error!", "Only numeric data can be transposed (except for the first column).");
			return;
		}
	}

	//create new header column
	QString header_col_header = data_.column(0).header();
	QVector<QString> header_col;
	header_col.reserve(data_.columnCount()-1);
	for (int c=1; c<data_.columnCount(); ++c)
	{
		header_col.append(data_.column(c).header());
	}

	//create new headers
	QVector<QString> headers;
	headers.reserve(data_.rowCount());
	for (int r=0; r<data_.rowCount(); ++r)
	{
		headers.append(data_.column(0).string(r));
	}


	//create new data columns
	QVector< QVector<double> > cols;
	cols.reserve(data_.rowCount());
	for (int r=0; r<data_.rowCount(); ++r)
	{
		QVector<double> col;
		col.reserve(data_.columnCount());
		for (int c=1; c<data_.columnCount(); ++c)
		{
			col.append(data_.numericColumn(c).value(r));
		}
		cols.append(col);
	}

	//update dataset and GUI
	data_.clear(false);
	data_.addColumn(header_col_header, header_col);
	for (int c=0; c<cols.count(); ++c)
	{
		data_.addColumn(headers[c], cols[c]);
	}
	grid_->render();
}


void MainWindow::tableContextMenu(QPoint point)
{
	QMenu* main_menu = grid_->createStandardContextMenu();

	if (grid_->selectionInfo().isColumnSelection)
	{
		//overall and selected columns count
		QList<int> selected = grid_->selectedColumns();
		int selected_count = selected.size();
		int text_count = 0;
		for (int i=0; i<selected.size(); ++i)
		{
			text_count += (data_.column(selected[i]).type()==BaseColumn::STRING);
		}

		//separator
		main_menu->addSeparator();

		//statistics
		QAction* action = main_menu->addAction("Basic statistics", this, SLOT(basicStatistics()));
		action->setEnabled(selected_count==1 && text_count==0);

		//plots
		QMenu* menu = main_menu->addMenu("Plots");
		menu->setEnabled(selected_count>0 && text_count==0);
		action = menu->addAction(QIcon(":/Icons/Histogram.png"), "Histogram", this, SLOT(histogram()));
		action->setEnabled(selected_count==1);
		action = menu->addAction(QIcon(":/Icons/Scatterplot.png"), "Scatter plot", this, SLOT(scatterPlot()));
		action->setEnabled(selected_count==2);
		action = menu->addAction(QIcon(":/Icons/Lineplot.png"), "Plot", this, SLOT(dataPlot()));
		action->setEnabled(selected_count>0);
		action = menu->addAction(QIcon(":/Icons/Boxplot.png"), "Box plot", this, SLOT(boxPlot()));
		action->setEnabled(selected_count>0);

		//signal processing
		menu = main_menu->addMenu("Signal processing");
		menu->setEnabled(selected_count==1 && text_count==0);
		action = menu->addAction("Moving average", this, SLOT(smoothAverage()));
		action = menu->addAction("Moving median", this, SLOT(smoothMedian()));
		action = menu->addAction("Savitzky-Golay", this, SLOT(smoothSavitzkyGolay()));
		action = menu->addAction("Bessel", this, SLOT(smoothBessel()));
	}

	//execute
	main_menu->exec(grid_->viewport()->mapToGlobal(point));
	delete main_menu;
}

void MainWindow::smoothAverage()
{
	smooth_(Smoothing::MovingAverage, "_ma");
}

void MainWindow::smoothMedian()
{
	smooth_(Smoothing::MovingMedian, "_mm");
}

void MainWindow::smoothSavitzkyGolay()
{
	smooth_(Smoothing::SavitzkyGolay, "_sg");
}

void MainWindow::smoothBessel()
{
	smooth_(Smoothing::Bessel, "_be");
}

void MainWindow::smooth_(Smoothing::Type type, QString suffix)
{
	Parameters params = Smoothing::defaultParameters(type);
	if (!ParameterEditor::asDialog("Moving average parameters", params))
	{
		return;
	}

	int index = grid_->selectedColumns()[0];
	QString header = data_.column(index).header();
	QVector<double> dataset = data_.numericColumn(index).values();

	Smoothing::smooth(dataset, type, params);

	data_.addColumn(header + suffix, dataset);
}

QString MainWindow::fileNameLabel()
{
	if (file_.type==NONE)
	{
		return "";
	}

	return " (" + QFileInfo(file_.name).fileName() + ")";
}

void MainWindow::histogram()
{
	int index = grid_->selectedColumns()[0];

	HistogramPlot* hist = new HistogramPlot();
	hist->setData(data_, index, QFileInfo(file_.name).baseName());
	GUIHelper::showWidgetAsDialog(hist, "Histogram of '" + data_.column(index).headerOrIndex(index) + fileNameLabel(), false);
}

void MainWindow::basicStatistics()
{
	int index = grid_->selectedColumns()[0];

	StatisticsSummaryWidget* stats = new StatisticsSummaryWidget();
	stats->setData(data_.numericColumn(index).statistics(data_.getRowFilter()));
	GUIHelper::showWidgetAsDialog(stats, "Basic statistics of '" + data_.column(index).headerOrIndex(index) + fileNameLabel(), false);
}

void MainWindow::scatterPlot()
{
	int x = grid_->selectedColumns()[0];
	int y = grid_->selectedColumns()[1];

	ScatterPlot* plot = new ScatterPlot();
	plot->setData(data_, x, y, QFileInfo(file_.name).baseName());
	GUIHelper::showWidgetAsDialog(plot, "Scatterplot of '" + data_.column(x).headerOrIndex(x) + "' and '" + data_.column(y).headerOrIndex(y) + "'" + fileNameLabel(), false);
}

void MainWindow::dataPlot()
{
	DataPlot* plot = new DataPlot();
	plot->setData(data_, grid_->selectedColumns(), QFileInfo(file_.name).baseName());
	GUIHelper::showWidgetAsDialog(plot, "Plot" + fileNameLabel(), false);
}

void MainWindow::boxPlot()
{
	BoxPlot* plot = new BoxPlot();
	plot->setData(data_, grid_->selectedColumns(), QFileInfo(file_.name).baseName());
	GUIHelper::showWidgetAsDialog(plot, "BoxPlot" + fileNameLabel(), false);
}

void MainWindow::on_about_triggered(bool /*checked*/)
{
	QMessageBox::about(this, "About " + QApplication::applicationName(), QApplication::applicationName() + " " + QApplication::applicationVersion() +"\n\nThis program is free software.\n\nThis program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.\n\nThis program is based in part on the work of the Qwt project (http://qwt.sf.net).");
}

void MainWindow::on_changelog_triggered(bool /*checked*/)
{
	ScrollableTextDialog dlg(this);
	dlg.setWindowTitle("ChangeLog " + QApplication::applicationName());
	dlg.setText(Helper::loadTextFile("://Resources/ChangeLog.html").join("\n"));
	dlg.exec();
}

void MainWindow::addToRecentFiles_(QString filename, FileType type)
{
	QString filename_and_type = filename + "|" + QString::number(type);

	recent_files_.removeAll(filename_and_type);

	if (QFile::exists(filename))
	{
		recent_files_.prepend(filename_and_type);
	}

	while (recent_files_.size() > 10)
	{
		recent_files_.removeLast();
	}

	Settings::setStringList("recent_files", recent_files_);
	updateRecentFilesMenu_();
}

void MainWindow::updateRecentFilesMenu_()
{
	QMenu* menu = new QMenu();

	for (int i=0; i<recent_files_.size(); ++i)
	{
		QStringList parts = recent_files_[i].split("|");

		QAction* action = menu->addAction(parts[0], this, SLOT(openRecentFile()));
		action->setData(parts[1].toInt());
	}

	ui_.recentlyOpened->setMenu(menu);
}

void MainWindow::openRecentFile()
{
	storeModifiedDataset_();

	QAction* action = qobject_cast<QAction*>(sender());

	openFile_(action->text(), (FileType)action->data().toInt());
}

void MainWindow::on_toggleColumnIndex_triggered(bool)
{
	bool show_cols = Settings::boolean("show_column_index");

	Settings::setBoolean("show_column_index", !show_cols);

	grid_->renderHeaders();
}

void MainWindow::on_toggleRowColors_triggered(bool)
{
	grid_->setAlternatingRowColors(!grid_->alternatingRowColors());
}

void MainWindow::on_showComments_triggered(bool)
{
	QTextBrowser browser;
	browser.setText(data_.comments().join("\n"));
	browser.setReadOnly(true);
	browser.setLineWrapMode(QTextBrowser::NoWrap);
	GUIHelper::showWidgetAsDialog(&browser, "Comments", false);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Delete && e->modifiers()==Qt::NoModifier)
	{
		grid_->removeSelectedColumns();
	}
	else if (e->key()==Qt::Key_F3 && e->modifiers()==Qt::NoModifier)
	{
		findNext();
	}
	else
	{
		QMainWindow::keyPressEvent(e);
	}
}

void MainWindow::goToRow(int row)
{
	row -= 1;

	// abort if not found
	if (row>=data_.rowCount())
	{
		statusBar()->showMessage(QString("Row with index '%1' does not exist!").arg(row+1), 5000);
		return;
	}

	grid_->scrollToItem(grid_->item(row, 0));
}

void MainWindow::findText(QString text, Qt::CaseSensitivity case_sensitive, DataGrid::FindType type)
{
	//clear selection
	grid_->clearSelection();

	//clear last search info
	find_.items.clear();
	find_.last = 0;

	//abort if text is empty
	if (text=="")
	{
		return;
	}

	//search
	find_.items = grid_->findItems(text, case_sensitive, type);

	// abort if not found
	if (find_.items.count()==0)
	{
		statusBar()->showMessage(QString("Search string '%1' not found!").arg(text), 5000);
		return;
	}

	// determine column and row
	int col = find_.items[0].first;
	int row = find_.items[0].second;

	// scroll
	QTableWidgetItem* item = grid_->item(row, col);
	grid_->scrollToItem(item);

	// set selection
	grid_->setRangeSelected(QTableWidgetSelectionRange(row, col, row, col), true);
}

void MainWindow::findNext()
{
	//clear selection
	grid_->clearSelection();

	//abort reasons
	if(find_.items.count() == 0)
	{
		return;
	}
	if(find_.last >= find_.items.count()-1)
	{
		statusBar()->showMessage(QString("Search hit the bottom!"), 5000);
		find_.last = -1;
		return;
	}

	// determine column and row
	find_.last += 1;
	int col = find_.items[find_.last].first;
	int row = find_.items[find_.last].second;

	// scroll
	QTableWidgetItem* item = grid_->item(row, col);
	grid_->scrollToItem(item);

	// set selection
	grid_->setRangeSelected(QTableWidgetSelectionRange(row, col, row, col), true);
}

void MainWindow::toggleFilter(bool enabled)
{
	data_.setFiltersEnabled(enabled);
	grid_->render();
}

void MainWindow::updateFilters()
{
	filter_widget_->renderFilters(data_);

	//auto-show/hide filter widget
	if (!data_.filtersPresent())
	{
		filter_widget_->hide();
	}
	else
	{
		filter_widget_->show();
	}
}

void MainWindow::updateInfoWidget()
{
	QString filter_info;
	if (data_.rowCount()!=grid_->rowCount())
	{
		filter_info = " (of " + QString::number(data_.rowCount()) + ")";
	}

	info_widget_->setText("cols: " + QString::number(data_.columnCount()) + " rows: " + QString::number(grid_->rowCount()) + filter_info);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasFormat("text/uri-list") && e->mimeData()->urls().count()==1)
	{
		e->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent* e)
{
	QString filename = e->mimeData()->urls().first().toLocalFile();
	if (filename.isEmpty())
	{
		return;
	}

	openFile_(filename, getType(filename), true);
}

void MainWindow::fileChanged()
{
	//do nothing if the file no longer exists
	if (!QFile::exists(file_.name)) return;

	//disable file watcher (to avaoid several opended reload dialogs)
	file_watcher_.clearFile();

	//show message box
	QMessageBox::StandardButton button = QMessageBox::information(this, "File changed", "The file '" + file_.name + "' was changed outside of TSVview.\nDo you want to re-load the file from disk?", QMessageBox::Yes, QMessageBox::No);

	//re-enable file watcher
	file_watcher_.setFile(file_.name);

	//reload file if dialog is accepted
	if (button == QMessageBox::Yes)
	{
		openFile_(file_.name, file_.type, false);
	}
}

MainWindow::FileType MainWindow::getType(QString filename)
{
	if (filename.endsWith(".zxv"))
	{
		return ZXV;
	}
	else if (filename.endsWith(".xml"))
	{
		return XML;
	}

	return TXT;
}

void MainWindow::setFile(QString name, MainWindow::FileType type, Parameters param)
{
	file_.name = name;
	file_.type = type;
	file_.param = param;

	//update recent files
	if (type!=NONE)
	{
		addToRecentFiles_(name, type);
	}

	//update file watcher
	file_watcher_.clearFile();
	if (type!=NONE)
	{
		file_watcher_.setFile(file_.name);
	}

	//update window title
	updateWindowTitle_();
}
