#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "GoToDockWidget.h"
#include "FindDockWidget.h"
#include "DelayedInitializationTimer.h"
#include "ui_MainWindow.h"
#include "DataGrid.h"
#include "Smoothing.h"
#include "DataSet.h"
#include "FilterWidget.h"
#include "FileWatcher.h"

class MainWindow
		: public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);

public slots:
	void delayedInitialization();
	void on_about_triggered(bool checked = false);
	void on_clearSettings_triggered(bool checked = false);
	void on_addToContext_triggered(bool checked = false);
	void on_transpose_triggered(bool checked = false);
	void on_grep_triggered(bool checked = false);
	void on_exit_triggered(bool checked = false);
	void on_newFile_triggered(bool);
    void on_openTsvFile_triggered(bool);
    void on_actionImportTxtFile_triggered(bool);
	void on_saveFile_triggered(bool);
    void on_actionSaveAs_triggered(bool);
    void on_actionExportHTML_triggered(bool);
    void on_actionExportCSV_triggered(bool);
    void on_resizeColumnWidth_triggered(bool);
	void on_resizeColumnHeightMinimum_triggered(bool);
	void on_resizeColumnHeight_triggered(bool);
	void on_goToRow_triggered(bool);
	void on_findText_triggered(bool);
	void on_filter_triggered(bool);
	void on_toggleColumnIndex_triggered(bool);
	void on_toggleRowColors_triggered(bool);
	void on_showComments_triggered(bool);
	void on_fileNameToClipboard_triggered(bool);
    void on_fileFolderInExplorer_triggered(bool);
    void on_actionShowDatasetInfo_triggered(bool);
    void on_actionGenerateExampleData_triggered(bool);
	void tableContextMenu(QPoint point);

	QString fileNameLabel();
	void histogram();
	void basicStatistics();
	void scatterPlot();
	void dataPlot();
	void boxPlot();

	void smoothAverage();
	void smoothMedian();
	void smoothSavitzkyGolay();

	void goToRow(int row);
	void findText(QString text, Qt::CaseSensitivity case_sensitive, DataGrid::FindType type);
	void findNext();
	void datasetStatusChanged(bool status);
	void updateInfoWidget();

	void openRecentFile();
	void toggleFilter(bool enabled);
	void updateFilters();

	void dragEnterEvent(QDragEnterEvent* e);
	void dropEvent(QDropEvent* e);

	void fileChanged();

protected:
	virtual void keyPressEvent(QKeyEvent* event);

private:
	Ui::MainWindow ui_;
	DelayedInitializationTimer delayed_init_timer_;
	DataSet data_;
    bool debug_;

	QStringList recent_files_;
	GoToDockWidget* goto_widget_;
	FilterWidget* filter_widget_;

	///Open file struct
    QString filename_;
	FileWatcher file_watcher_;

	/// Last search struct
	struct
	{
		QVector< QPair<int, int> > items;
		int last;
	}
	find_;
	FindDockWidget* find_widget_;

	QLabel* info_widget_;

	void smooth_(Smoothing::Type type, QString suffix);
	void updateWindowTitle_();
	void closeEvent(QCloseEvent* event);
    void openFile_(QString filename, bool remember_path=true, bool show_import_dialog=false);
	void storeModifiedDataset_();
	void addToRecentFiles_(QString filename);
	void updateRecentFilesMenu_();
    void setFile(QString name);
    static bool isTsv(QString filename);
};

#endif
