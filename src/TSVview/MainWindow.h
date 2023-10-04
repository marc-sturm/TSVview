#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "GoToDockWidget.h"
#include "FindDockWidget.h"

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
	void on_about_triggered(bool checked = false);
	void on_clearSettings_triggered(bool checked = false);
	void on_addToContext_triggered(bool checked = false);
	void on_transpose_triggered(bool checked = false);
	void on_exit_triggered(bool checked = false);
	void on_newFile_triggered(bool);
	void on_openZXV_triggered(bool);
	void on_openXML_triggered(bool);
	void on_openTXT_triggered(bool);
	void on_saveFile_triggered(bool);
	void on_saveFileAs_triggered(bool);
	void on_resizeColumns_triggered(bool);
	void on_resizeRows_triggered(bool);
	void on_goToRow_triggered(bool);
	void on_findText_triggered(bool);
	void on_filter_triggered(bool);
	void on_toggleColumnIndex_triggered(bool);
	void on_toggleRowColors_triggered(bool);
	void on_showComments_triggered(bool);

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
	void smoothBessel();

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
	DataSet data_;

	DataGrid* grid_;
	QStringList recent_files_;
	GoToDockWidget* goto_widget_;
	FilterWidget* filter_widget_;

	///File type enum
	enum FileType
		{
		TXT,
		XML, //TODO remove this format as well?
		ZXV, //TODO remove this format
		NONE
		};

	///Open file struct
	struct
	{
		QString name;
		FileType type;
		Parameters param;
	}
	file_;

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
	void openFile_(QString filename, FileType type, bool remember_path = true);
	void storeModifiedDataset_();
	void addToRecentFiles_(QString filename, FileType type);
	void updateRecentFilesMenu_();
	static FileType getType(QString filename);
	void setFile(QString name, FileType type = NONE, Parameters param = Parameters());
};

#endif
