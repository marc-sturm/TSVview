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
#include <QSpacerItem>
#include "BasePlot.h"
#include "Settings.h"

BasePlot::BasePlot(QWidget *parent)
	: QWidget(parent)
	, params_()
	, chart_view_(nullptr)
	, chart_(nullptr)
	, editor_(nullptr)
	, x_label_(nullptr)
	, y_label_(nullptr)
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

	//create plot and add it chart_view_yout
	QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	policy.setHorizontalStretch(100);
	policy.setVerticalStretch(100);
	chart_view_ = new MyChartView();
	chart_view_->setSizePolicy(policy);
	chart_view_->setRenderHint(QPainter::Antialiasing, true);
	chart_view_->setBackgroundBrush(Qt::white);
	chart_view_->setRubberBand(QChartView::RectangleRubberBand);
	connect(chart_view_, SIGNAL(resetZoom()), this, SLOT(resetZoom()));
	connect(chart_view_, SIGNAL(zoomIn()), this, SLOT(zoomIn()));
	layout->addWidget(chart_view_, 0, 2);

	//add settings button
	QToolButton* button = new QToolButton();
	button->setIcon(QIcon(":/Icons/Settings.png"));
	button->setToolTip("Show/hide settings");
	connect(button, SIGNAL(clicked()), this, SLOT(showSettings()));
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

void BasePlot::showSettings()
{
	editor_->setVisible(!editor_->isVisible());
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
	QPixmap image(chart_view_->size());
	image.fill(Qt::white);

	//print plot to image
	QPainter painter(&image);
	chart_view_->render(&painter);

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
		QPixmap image(chart_view_->width(), chart_view_->height());
		image.fill(Qt::white);

		//print plot to image
		QPainter painter(&image);
		chart_view_->render(&painter);

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

		QPainter painter(&image);
		chart_view_->render(&painter);

		//store the last used path
		Settings::setPath("path_images", filename);
	}
}

void BasePlot::toggleSeriesVisibility()
{
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	QAbstractSeries* series = marker->series();

	//toggle visibility
	if(series->isVisible())
	{
		series->setVisible(false);
		marker->setVisible(true);
	}
	else
	{
		series->setVisible(true);
	}

	//shade legend marker according to visibility
	double alpha = series->isVisible() ? 1.0 : 0.5;

	QBrush brush = marker->labelBrush();
	QColor color = brush.color();
	color.setAlphaF(alpha);
	brush.setColor(color);
	marker->setLabelBrush(brush);

	brush = marker->brush();
	color = brush.color();
	color.setAlphaF(alpha);
	brush.setColor(color);
	marker->setBrush(brush);

	QPen pen = marker->pen();
	color = pen.color();
	color.setAlphaF(alpha);
	pen.setColor(color);
	marker->setPen(pen);
}

void BasePlot::resetZoom()
{
	chart_->zoomReset();
}

void BasePlot::zoomIn()
{
	chart_->zoomIn();
}

void BasePlot::enableMouseTracking()
{
	chart_view_->enableMouseTracking();
	if (y_label_==nullptr)
	{
		QBoxLayout* status_bar_layout =  new QBoxLayout(QBoxLayout::LeftToRight);
		QGridLayout* main_layout = (QGridLayout*)layout();
		main_layout->addLayout(status_bar_layout, 1, 2, Qt::AlignHCenter);

		//X axis
		x_label_ = new QLabel();
		x_label_->setMinimumWidth(70);
		status_bar_layout->addWidget(x_label_);
		connect(chart_view_, SIGNAL(xPosition(QString)), x_label_, SLOT(setText(QString)));

		//Y axis
		y_label_ = new QLabel();
		y_label_->setMinimumWidth(70);
		status_bar_layout->addWidget(y_label_);
		connect(chart_view_, SIGNAL(yPosition(QString)), y_label_, SLOT(setText(QString)));

		status_bar_layout->addStretch(100);

		//additional info
		info_label_ = new QLabel();
		info_label_->setMinimumWidth(70);
		status_bar_layout->addWidget(info_label_);
	}
}

QAbstractSeries* BasePlot::search(QString name) const
{
	foreach(QAbstractSeries* series, chart_->series())
	{
		if (series->name()==name) return series;
	}

	return nullptr;
}
