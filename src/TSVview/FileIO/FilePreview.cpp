#include "FilePreview.h"
#include "VersatileTextStream.h"


FilePreview::FilePreview(QString filename, QString display_name, QWidget *parent)
	: QDialog(parent)
	, ui_()
    , filename_(filename)
    , display_name_(display_name)
{
	ui_.setupUi(this);

    setWindowTitle("File preview: " + display_name);

	connect(ui_.reload_button, SIGNAL(clicked()), this, SLOT(showPreview()));

	showPreview();
}

void FilePreview::showPreview()
{
	// load first n lines of file
	int lines = ui_.lines->value();

	int i=0;
    QString text;
    VersatileTextStream stream(filename_);
    while (!stream.atEnd() && i<lines)
	{
        text += stream.readLine() + "\n";
		++i;
	}

	// set text browser text
	ui_.text_browser->setPlainText(text);
}
