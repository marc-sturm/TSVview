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
#include "TextImportPreview.h"
#include "StatisticsSummaryWidget.h"
#include "DataPlot.h"
#include "Settings.h"
#include "Smoothing.h"
#include "GUIHelper.h"
#include "ScatterPlot.h"
#include "HistogramPlot.h"
#include "BoxPlot.h"
#include <QStyleFactory>
#include <QLibraryInfo>
#include <QProcessEnvironment>
#include "Helper.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, delayed_init_timer_(this, true)
	, data_()
    , debug_(QProcessEnvironment::systemEnvironment().contains("QTDIR"))
	, recent_files_()
{
	ui_.setupUi(this);
    ui_.menuDebug->setVisible(debug_);
	QApplication::setStyle(QStyleFactory::create("windowsvista"));

	//create info widget in status bar
	info_widget_ = new QLabel("cols: 0 rows: 0");
	statusBar()->addPermanentWidget(info_widget_);

	//create grid and dataset
	connect(ui_.grid, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenu(QPoint)));
	connect(ui_.grid, SIGNAL(rendered()), this, SLOT(updateInfoWidget()));
	connect(&data_, SIGNAL(modificationStatusChanged(bool)), this, SLOT(datasetStatusChanged(bool)));
	connect(&data_, SIGNAL(filtersChanged()), this, SLOT(updateFilters()));
    connect(&data_, SIGNAL(headersChanged()), this, SLOT(updateFilters()));
	ui_.grid->setData(data_);

	recent_files_ = Settings::stringList("recent_files", true);
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
	connect(filter_widget_, SIGNAL(editFilter(int)), ui_.grid, SLOT(editFilter(int)));
	connect(filter_widget_, SIGNAL(removeFilter(int)), ui_.grid, SLOT(removeFilter(int)));
	connect(filter_widget_, SIGNAL(removeAllFilters()), ui_.grid, SLOT(removeAllFilters()));
	connect(filter_widget_, SIGNAL(reduceToFiltered()), ui_.grid, SLOT(reduceToFiltered()));
	connect(filter_widget_, SIGNAL(loadFilter()), ui_.grid, SLOT(loadFilter()));
	connect(filter_widget_, SIGNAL(storeFilter()), ui_.grid, SLOT(storeFilter()));
	connect(filter_widget_, SIGNAL(deleteFilter()), ui_.grid, SLOT(deleteFilter()));

	//enable drop event
	setAcceptDrops(true);

	//init file
    setFile("");
	connect(&file_watcher_, SIGNAL(fileChanged()), this, SLOT(fileChanged()));
}

void MainWindow::delayedInitialization()
{
	//load argument file
	if (QApplication::arguments().count()==2)
	{
		QString filename = QApplication::arguments().at(1);
		openFile_(filename);
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
    if (data_.modified() && data_.columnCount()!=0)
	{
		QMessageBox box(this);
		box.setWindowTitle(QApplication::applicationName());
        box.setText("Save changes to '" + filename_ + "'?");
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
    QString title = QApplication::applicationName() + " - " + (filename_.isEmpty() ? "untitled" : filename_);
	if (data_.modified())
	{
		title += "*";
	}
	setWindowTitle(title);
}

void MainWindow::on_newFile_triggered(bool)
{
	storeModifiedDataset_();

    setFile("");
	data_.clear(true);
}

void MainWindow::on_openTsvFile_triggered(bool)
{
	storeModifiedDataset_();

    QString filename = QFileDialog::getOpenFileName(this, "Open TSV file", Settings::path("path_open", true), "TSV files (*.tsv *.tsv.gz)");
    if (filename.isEmpty()) return;

	openFile_(filename);
}

void MainWindow::on_actionImportTxtFile_triggered(bool)
{
    storeModifiedDataset_();

    QString filename = QFileDialog::getOpenFileName(this, "Inport text file", Settings::path("path_open", true), "All files(*.*);;All files(*.csv);;Txt files(*.txt)");
    if (filename.isEmpty()) return;

    openFile_(filename, true, true);
}

void MainWindow::openFile_(QString filename, bool remember_path, bool show_import_dialog)
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

    //update open path in settings
    if (remember_path) Settings::setPath("path_open", filename);

	//reset GUI
    setFile("");

	//load file
    data_.blockSignals(true);
    try
    {
        if (show_import_dialog || !isTsv(filename))
        {
            TextImportPreview preview(filename, filename, filename.endsWith(".csv") ,this);
            if(preview.exec())
            {
                data_.import(filename, filename, preview.parameters());
            }
		}
        else
        {
            data_.load(filename, filename);
            setFile(filename);
        }
    }
	catch (Exception& e)
	{
		QMessageBox::warning(this, "Error loading file.", e.message());
    }

	//update GUI
	updateFilters();
    ui_.grid->render();

	//resize
    on_resizeColumnWidth_triggered(true);
    on_resizeColumnHeight_triggered(true);

	//re-enable data signals
    data_.blockSignals(false);
}

void MainWindow::on_saveFile_triggered(bool)
{
    //no filename > get it from the user
    if (filename_.isEmpty())
    {
        QString filename = QFileDialog::getSaveFileName(this, "Save file", Settings::path("path_open", true) + filename_, "TSV files (*.tsv *.tsv.gz)");
        if (filename.isEmpty()) return;

        filename_ = filename;
        Settings::setPath("path_open", filename_);
	}

    //disable file watcher
    file_watcher_.clearFile();

    //store
	try
	{
        data_.store(filename_);
	}
    catch (FileAccessException& e)
	{
        QMessageBox::warning(this, "Error storing file '"+filename_ + "'", e.message());
	}

    //update name (and add to recent files)
    setFile(filename_);

	//re-enable file watcher
    file_watcher_.setFile(filename_);

	data_.setModified(false);
}

void MainWindow::on_actionSaveAs_triggered(bool)
{
    if (data_.columnCount()==0) return;

    QString filename = QFileDialog::getSaveFileName(this, "Save as",  filename_, "TSV files (*.tsv *.tsv.gz);;All files (*.*)");
    data_.store(filename);
    setFile(filename);
}

void MainWindow::on_actionExportHTML_triggered(bool)
{
    if (data_.columnCount()==0) return;

    QString filename = QFileDialog::getSaveFileName(this, "Export as HTML",  filename_+".html", "HTML files (*.html);;All files (*.*)");
    data_.storeAs(filename, ExportFormat::HTML);
}

void MainWindow::on_actionExportCSV_triggered(bool)
{
    if (data_.columnCount()==0) return;

    QString filename = QFileDialog::getSaveFileName(this, "Export as CSV",  filename_+".csv", "CSV files (*.csv);;All files (*.*)");
    data_.storeAs(filename, ExportFormat::CSV);
}

void  MainWindow::on_resizeColumnWidth_triggered(bool)
{
    ui_.grid->resizeColumnWidth();
}


void  MainWindow::on_resizeColumnHeight_triggered(bool)
{
    ui_.grid->resizeColumnHeight();
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
    const int col_count = data_.columnCount();
    if (col_count==0) return;

	//check that all columns are numeric
    for (int i=1; i<col_count; ++i)
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
    header_col.reserve(col_count-1);
    for (int c=1; c<col_count; ++c)
	{
        header_col << data_.column(c).header();
	}

	//create new headers
	QVector<QString> headers;
	headers.reserve(data_.rowCount());
	for (int r=0; r<data_.rowCount(); ++r)
	{
        headers << data_.column(0).string(r);
	}

	//create new data columns
	QVector< QVector<double> > cols;
    QVector< QVector<char> > decimals;
	cols.reserve(data_.rowCount());
	for (int r=0; r<data_.rowCount(); ++r)
	{
		QVector<double> col;
        col.reserve(col_count);
        QVector<char> dec;
        dec.reserve(col_count);
        for (int c=1; c<col_count; ++c)
		{
            col << data_.numericColumn(c).value(r);
            dec << data_.numericColumn(c).decimals(r);
		}
        cols << col;
        decimals << dec;
	}

	//update dataset and GUI
	data_.clear(false);
	data_.addColumn(header_col_header, header_col);
	for (int c=0; c<cols.count(); ++c)
	{
		data_.blockSignals(true);
        data_.addColumn(headers[c], cols[c], decimals[c]);
		data_.blockSignals(false);
	}
	ui_.grid->render();
}

void MainWindow::on_grep_triggered(bool /*checked*/)
{
	ui_.grid->grepLines();
}


void MainWindow::tableContextMenu(QPoint point)
{
	QMenu* main_menu = ui_.grid->createStandardContextMenu();

	if (ui_.grid->selectionInfo().isColumnSelection)
	{
		//overall and selected columns count
		QList<int> selected = ui_.grid->selectedColumns();
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
        menu = main_menu->addMenu("Smoothing");
		menu->setEnabled(selected_count==1 && text_count==0);
		menu->addAction("Moving average", this, SLOT(smoothAverage()));
		menu->addAction("Moving median", this, SLOT(smoothMedian()));
		menu->addAction("Savitzky-Golay", this, SLOT(smoothSavitzkyGolay()));
	}

	//execute
	main_menu->exec(ui_.grid->viewport()->mapToGlobal(point));
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

void MainWindow::smooth_(Smoothing::Type type, QString suffix)
{
	Parameters params = Smoothing::defaultParameters(type);
	if (!ParameterEditor::asDialog(this->windowIcon(), "Moving average parameters", params))
	{
		return;
	}

	int index = ui_.grid->selectedColumns().at(0);
	QString header = data_.column(index).header();
	QVector<double> dataset = data_.numericColumn(index).values();

	Smoothing::smooth(dataset, type, params);

    data_.addColumn(header + suffix, dataset, data_.numericColumn(index).decimals());
}

QString MainWindow::fileNameLabel()
{
    if (filename_.isEmpty())
	{
		return "";
	}

    return " (" + QFileInfo(filename_).fileName() + ")";
}

void MainWindow::histogram()
{
	int index = ui_.grid->selectedColumns().at(0);

	HistogramPlot* hist = new HistogramPlot();
    hist->setData(data_, index, QFileInfo(filename_).baseName());
	auto dlg = GUIHelper::createDialog(hist, "Histogram of '" + data_.column(index).headerOrIndex(index) + fileNameLabel());
	dlg->exec();
}

void MainWindow::basicStatistics()
{
	int index = ui_.grid->selectedColumns().at(0);

	StatisticsSummaryWidget* stats = new StatisticsSummaryWidget();
	stats->setData(data_.numericColumn(index).statistics(data_.getRowFilter()));
	auto dlg = GUIHelper::createDialog(stats, "Basic statistics of '" + data_.column(index).headerOrIndex(index) + fileNameLabel());
	dlg->exec();
}

void MainWindow::scatterPlot()
{
	int x = ui_.grid->selectedColumns().at(0);
	int y = ui_.grid->selectedColumns().at(1);
	ScatterPlot* plot = new ScatterPlot();
    plot->setData(data_, x, y, QFileInfo(filename_).baseName());
	auto dlg = GUIHelper::createDialog(plot, "Scatterplot of '" + data_.column(x).headerOrIndex(x) + "' and '" + data_.column(y).headerOrIndex(y) + "'" + fileNameLabel());
	dlg->exec();
}

void MainWindow::dataPlot()
{
	DataPlot* plot = new DataPlot();
    plot->setData(data_, ui_.grid->selectedColumns(), QFileInfo(filename_).baseName());
	auto dlg = GUIHelper::createDialog(plot, "Plot" + fileNameLabel());
	dlg->exec();
}

void MainWindow::boxPlot()
{
	BoxPlot* plot = new BoxPlot();
    plot->setData(data_, ui_.grid->selectedColumns(), QFileInfo(filename_).baseName());
	auto dlg = GUIHelper::createDialog(plot, "BoxPlot" + fileNameLabel());
	dlg->exec();
}

void MainWindow::on_about_triggered(bool /*checked*/)
{
	QString about_text = QApplication::applicationName() + " " + QApplication::applicationVersion();

	about_text += "\n\n";
	about_text += "A free TSV viewer.";

    about_text += "\n\n";
	about_text += "Architecture: " + QSysInfo::buildCpuArchitecture() + "\n";
	about_text += "Qt version: " + QLibraryInfo::version().toString() + "\n";

	QMessageBox::about(this, QApplication::applicationName(), about_text);
}

void MainWindow::addToRecentFiles_(QString filename)
{
	recent_files_.removeAll(filename);

	if (QFile::exists(filename))
	{
		recent_files_.prepend(filename);
	}

    if (recent_files_.size() > 20)
	{
        recent_files_.resize(20);
	}

	Settings::setStringList("recent_files", recent_files_);
	updateRecentFilesMenu_();
}

void MainWindow::updateRecentFilesMenu_()
{
	QMenu* menu = new QMenu();
	for (int i=0; i<recent_files_.size(); ++i)
	{
		menu->addAction(recent_files_[i], this, SLOT(openRecentFile()));
	}
	ui_.recentlyOpened->setMenu(menu);
}

void MainWindow::openRecentFile()
{
	storeModifiedDataset_();

	QAction* action = qobject_cast<QAction*>(sender());

	openFile_(action->text());
}

void MainWindow::on_toggleColumnIndex_triggered(bool)
{
	bool show_cols = Settings::contains("show_column_index") && Settings::boolean("show_column_index");

	Settings::setBoolean("show_column_index", !show_cols);

	ui_.grid->renderHeaders();
}

void MainWindow::on_toggleRowColors_triggered(bool)
{
	ui_.grid->setAlternatingRowColors(!ui_.grid->alternatingRowColors());
}

void MainWindow::on_showComments_triggered(bool)
{
	QTextBrowser* browser = new QTextBrowser();
	browser->setText(data_.comments().join("\n"));
	browser->setReadOnly(true);
	browser->setLineWrapMode(QTextBrowser::NoWrap);
    browser->setMinimumSize(800, 600);
	auto dlg = GUIHelper::createDialog(browser, "Comments");
	dlg->exec();
}

void MainWindow::on_fileNameToClipboard_triggered(bool)
{
    QApplication::clipboard()->setText(filename_);
}

void MainWindow::on_fileFolderInExplorer_triggered(bool)
{
    if (filename_.isEmpty()) return;

    QDesktopServices::openUrl(QUrl(QFileInfo(filename_).path()));
}

void MainWindow::on_actionShowDatasetInfo_triggered(bool)
{
    int col_count = data_.columnCount();
    qDebug() << "cols:" << col_count << "rows:" << data_.rowCount() << "modified:" << data_.modified();
    for (int c=0; c<col_count; ++c)
    {
        QString col_text = "colum " + QString::number(c) + ": type=" + BaseColumn::typeToString(data_.column(c).type());
        qDebug() << col_text;
    }
}

void MainWindow::on_actionGenerateExampleData_triggered(bool)
{
    int rows = QInputDialog::getInt(this, "Generate example data", "rows:", 0);
    if (rows<1) return;

    DataSet tmp;

    //add string column
    QVector<QString> c1;
    for(int rep=0; rep<5; ++rep)
    {
        c1.clear();
        for(int i=0; i<rows; ++i)
        {
            c1 << Helper::randomString(4);
        }
        tmp.addColumn("col_string"+QString::number(rep+1), c1);
    }

    //add int column
    QVector<double> c2;
    c2.reserve(rows);
    for(int i=0; i<rows; ++i)
    {
        c2 << (int)std::round(Helper::randomNumber(0, 10000));
    }
    tmp.addColumn("col_int", c2, QVector<char>(rows, 0));

    //add float column
    c2.clear();
    for(int i=0; i<rows; ++i)
    {
        c2 << Helper::randomNumber(0, 100);
    }
    tmp.addColumn("col_float", c2, QVector<char>(rows, 2));

    //store
    tmp.store(QApplication::applicationDirPath() + "/example_data.tsv");
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Delete && e->modifiers()==Qt::NoModifier)
	{
		ui_.grid->removeSelectedColumns();
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

	ui_.grid->scrollToItem(ui_.grid->item(row, 0));
    ui_.grid->selectRow(row);
}

void MainWindow::findText(QString text, Qt::CaseSensitivity case_sensitive, DataGrid::FindType type)
{
	//clear selection
	ui_.grid->clearSelection();

	//clear last search info
	find_.items.clear();
	find_.last = 0;

	//abort if text is empty
	if (text=="")
	{
		return;
	}

	//search
	find_.items = ui_.grid->findItems(text, case_sensitive, type);

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
	QTableWidgetItem* item = ui_.grid->item(row, col);
	ui_.grid->scrollToItem(item);

	// set selection
	ui_.grid->setRangeSelected(QTableWidgetSelectionRange(row, col, row, col), true);
}

void MainWindow::findNext()
{
	//clear selection
	ui_.grid->clearSelection();

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
	QTableWidgetItem* item = ui_.grid->item(row, col);
	ui_.grid->scrollToItem(item);

	// set selection
	ui_.grid->setRangeSelected(QTableWidgetSelectionRange(row, col, row, col), true);
}

void MainWindow::toggleFilter(bool enabled)
{
	data_.setFiltersEnabled(enabled);
	ui_.grid->render();
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
	if (data_.rowCount()!=ui_.grid->rowCount())
	{
		filter_info = " (of " + QString::number(data_.rowCount()) + ")";
	}

	info_widget_->setText("cols: " + QString::number(data_.columnCount()) + " rows: " + QString::number(ui_.grid->rowCount()) + filter_info);
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

	openFile_(filename);
}

void MainWindow::fileChanged()
{
	//do nothing if the file no longer exists
    if (!QFile::exists(filename_)) return;

	//disable file watcher (to avaoid several opended reload dialogs)
	file_watcher_.clearFile();

	//show message box
    QMessageBox::StandardButton button = QMessageBox::information(this, "File changed", "The file '" + filename_ + "' was changed outside of TSVview.\nDo you want to re-load the file from disk?", QMessageBox::Yes, QMessageBox::No);

	//re-enable file watcher
    file_watcher_.setFile(filename_);

	//reload file if dialog is accepted
	if (button == QMessageBox::Yes)
	{
        openFile_(filename_, false);
	}
}

void MainWindow::setFile(QString name)
{
    filename_ = name;

	//update recent files
	if (!name.isEmpty())
	{
		addToRecentFiles_(name);
	}

	//update file watcher
	file_watcher_.clearFile();
	if (!name.isEmpty())
	{
        file_watcher_.setFile(filename_);
	}

	//update window title
	updateWindowTitle_();
}

bool MainWindow::isTsv(QString filename)
{
    filename = filename.toLower();
    return filename.endsWith(".tsv") || filename.endsWith(".tsv.gz");
}
