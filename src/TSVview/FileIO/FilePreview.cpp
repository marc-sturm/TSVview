#include <QFileInfo>
#include <QPushButton>
#include <QMenu>
#include <QInputDialog>
#include <QTextBrowser>

#include "FilePreview.h"
#include "DataSet.h"
#include "XMLFile.h"
#include "CustomExceptions.h"
#include "Settings.h"

FilePreview::FilePreview(QTextStream& stream, QString location, QWidget *parent)
	: QDialog(parent)
	, ui_()
	, stream_(stream)
	, location_(location)
{
	ui_.setupUi(this);

	setWindowTitle("File preview: " + QFileInfo(location).fileName());

	connect(ui_.reload_button, SIGNAL(clicked()), this, SLOT(showPreview()));

	showPreview();
}

void FilePreview::showPreview()
{
	// load first n lines of file
	int lines = ui_.lines->value();

	int i=0;
	QString text;
	stream_.seek(0);
	while (!stream_.atEnd() && i<lines)
	{
		text += stream_.readLine() + "\n";
		++i;
	}

	// set text browser text
	ui_.text_browser->setPlainText(text);
}
