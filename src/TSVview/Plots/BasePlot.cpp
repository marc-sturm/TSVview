#include <QBoxLayout>
#include <QToolButton>
#include <QPixmap>
#include <QPainter>
#include <QPicture>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QSvgGenerator>
#include <QDebug>
#include <QMouseEvent>
#include <QClipboard>

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>

#include "BasePlot.h"
#include "Settings.h"

BasePlot::BasePlot(QWidget *parent)
	: QWidget(parent)
	, params_()
	, plot_(0)
	, editor_(0)
	, x_label_(0)
	, y_label_(0)
{
	//general settings
	setMinimumWidth(500);
	setMinimumHeight(400);

	//create layout
	QGridLayout* layout = new QGridLayout();
	setLayout(layout);

	//create tool bar and add ot to layout
	toolbar_ = new QToolBar();
	toolbar_->setOrientation(Qt::Vertical);
	toolbar_->setIconSize(QSize(16, 16));
	layout->addWidget(toolbar_, 0, 0);

	//create parameter editor and add it to layout
	editor_ = new ParameterEditor(this);
	editor_->setVisible(false);
	layout->addWidget(editor_, 0, 1);

	//create plot and add it to layout
	plot_ = new QwtPlot();
	plot_->setCanvasBackground(Qt::white);
	QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	policy.setHorizontalStretch(100);
	policy.setVerticalStretch(100);
	plot_->setSizePolicy(policy);
	layout->addWidget(plot_, 0, 2);

	//add settings button
	QToolButton* button = new QToolButton();
	button->setIcon(QIcon(":/Icons/Settings.png"));
	button->setToolTip("Show/hide settings");
	connect(button, SIGNAL(clicked()), this, SLOT(showSettings_()));
	addToToolbar(button);
	addSeparatorToToolbar();

	//add print button (PNG)
	button = new QToolButton();
	button->setIcon(QIcon(":/Icons/SaveAsPng.png"));
	button->setToolTip("Save plot as PNG");
	button->setShortcut(QKeySequence::Save);
	connect(button, SIGNAL(clicked()), this, SLOT(saveAsPng()));
	addToToolbar(button);

	//add print button (SVG)
	button = new QToolButton();
	button->setIcon(QIcon(":/Icons/SaveAsSvg.png"));
	button->setToolTip("Save plot as SVG");
	connect(button, SIGNAL(clicked()), this, SLOT(saveAsSvg()));
	addToToolbar(button);

	//copy to clipboard
	button = new QToolButton();
	button->setIcon(QIcon(":/Icons/CopyToClipboard.png"));
	button->setToolTip("Copy to clipboard");
	button->setShortcut(QKeySequence::Copy);
	connect(button, SIGNAL(clicked()), this, SLOT(copyToClipboard()));
	addToToolbar(button);
}

void BasePlot::showSettings_()
{
	if (editor_->isVisible())
	{
		editor_->setVisible(false);
	}
	else
	{
		editor_->setVisible(true);
	}
}

void BasePlot::addToToolbar(QToolButton* button)
{
	toolbar_->setStyleSheet("QToolBar { border: 0px }");
	toolbar_->addWidget(button);
}

void BasePlot::addSeparatorToToolbar()
{
	QWidget* separator = new QWidget();
	separator->setMinimumHeight(6);
	separator->setMaximumHeight(6);
	toolbar_->addWidget(separator);
}


void BasePlot::copyToClipboard()
{
	//create empty image
	QPixmap image(2*plot_->width(), 2*plot_->height());
	image.fill(Qt::white);

	//print plot to image
	QwtPlotRenderer renderer;
	renderer.renderTo(plot_, image);

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setPixmap(image);
}

void BasePlot::saveAsPng()
{
	QString path = Settings::path("path_images", true);
	QString selected_filter = "PNG file (*.png)";
	QString filename = QFileDialog::getSaveFileName(this, "Save plot in PNG format",  path+filename_+".png", "All files (*.*);;"+selected_filter, &selected_filter);
	if (filename!="")
	{

		//create empty image
		QPixmap image(plot_->width(), plot_->height());
		image.fill(Qt::white);

		//print plot to image
		QwtPlotRenderer renderer;
		renderer.renderTo(plot_, image);

		//save image
		bool save_ok = image.save(filename, "PNG");
		if (save_ok)
		{
			Settings::setPath("path_images", filename);
		}
		else
		{
			QMessageBox::warning(this, "Error saving file!", "An unexpected error occured while saving the file!");
		}
	}
}

void BasePlot::saveAsSvg()
{
	QString path = Settings::path("path_images", true);
	QString selected_filter = "SVG file (*.svg)";
	QString filename = QFileDialog::getSaveFileName(this, "Save plot in SVG format",  path+filename_+".svg", "All files (*.*);;"+selected_filter, &selected_filter);
	if (filename!="")
	{
		//save image
		QSvgGenerator image;
		image.setFileName(filename);
		image.setSize(QSize(200, 200));
		image.setViewBox(QRect(0, 0, 200, 200));
		image.setTitle("SVG plot");
		image.setDescription("SVG plot");
		QwtPlotRenderer renderer;
		renderer.renderTo(plot_, image);

		//store the last used path
		Settings::setPath("path_images", filename);
	}
}


void BasePlot::enableMouseTracking(bool enabled)
{
	QGridLayout* main_layout = (QGridLayout*)layout();
	if (enabled)
	{
		//create status bar
		if (y_label_==0)
		{
			QBoxLayout* status_bar_layout =  new QBoxLayout(QBoxLayout::RightToLeft);
			main_layout->addLayout(status_bar_layout, 1, 0, 1, 3, Qt::AlignHCenter);

			//fill status bar layout with widgets
			y_label_ = new QLabel();
			y_label_->setMinimumWidth(70);
			status_bar_layout->addWidget(y_label_);
			x_label_ = new QLabel();
			x_label_->setMinimumWidth(70);
			status_bar_layout->addWidget(x_label_);
		}

		//enable mouse tracking
		plot_->canvas()->setMouseTracking(true);
		plot_->canvas()->installEventFilter(this);
	}
	else
	{
		plot_->canvas()->setMouseTracking(false);
		plot_->canvas()->removeEventFilter(this);
	}
}

bool BasePlot::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseMove)
	{
		QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
		double x = plot_->invTransform(QwtPlot::QwtPlot::xBottom, mouseEvent->x());
		x_label_->setText("x: " + QString::number(x));
		double y = plot_->invTransform(QwtPlot::QwtPlot::yLeft, mouseEvent->y());
		y_label_->setText("y: " + QString::number(y));

		return true;
	}
	if (event->type() == QEvent::Leave && y_label_!=0)
	{
		x_label_->setText("");
		y_label_->setText("");

		return true;
	}

	return QObject::eventFilter(obj, event);
}

