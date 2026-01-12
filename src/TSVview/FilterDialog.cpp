#include "FilterDialog.h"

#include <QMenu>
#include <QSet>
#include "cppCORE_global.h"

FilterDialog::FilterDialog(BaseColumn* column, QWidget* parent)
	: QDialog(parent)
	, column_(column)
	, ui_()
{
	ui_.setupUi(this);

	//populate list according to column type
	addOperation_(Filter::NONE);
	if (column_->type() == BaseColumn::NUMERIC)
	{
		addOperation_(Filter::FLOAT_EXACT);
		addOperation_(Filter::FLOAT_EXACT_NOT);
		addOperation_(Filter::FLOAT_LESS);
		addOperation_(Filter::FLOAT_LESS_EQUAL);
		addOperation_(Filter::FLOAT_GREATER);
		addOperation_(Filter::FLOAT_GREATER_EQUAL);
	}
	else
	{
		addOperation_(Filter::STRING_EXACT);
		addOperation_(Filter::STRING_EXACT_NOT);
		addOperation_(Filter::STRING_CONTAINS);
		addOperation_(Filter::STRING_CONTAINS_NOT);
		addOperation_(Filter::STRING_REGEXP);
		addOperation_(Filter::STRING_REGEXP_NOT);
	}

	//prepare dropdown list of texts
	if (column_->type() == BaseColumn::NUMERIC)
	{
		ui_.text_dropdown->hide();
	}
	else
	{
		ui_.text_dropdown->setMenu(new QMenu());
		connect(ui_.text_dropdown->menu(), SIGNAL(aboutToShow()), this, SLOT(updateDropdownText()));
	}

	//set validator (for float)
	if (column->type() == BaseColumn::NUMERIC)
	{
		QDoubleValidator* validator = new QDoubleValidator(this);
		validator->setLocale(QLocale::C);
		ui_.value->setValidator(validator);
	}

	//set current operation
	for (int i=0; i<ui_.operation->count(); ++i)
	{
		if (ui_.operation->itemData(i).toInt() == column->filter().type())
		{
			ui_.operation->setCurrentIndex(i);
		}
	}

	//if unset, select first operation (is/=)
	if (ui_.operation->currentIndex()==0)
	{
		ui_.operation->setCurrentIndex(1);
	}

	//set current value
	ui_.value->setText(column->filter().value());

	//connect slot
	connect(ui_.set, SIGNAL(clicked()), this, SLOT(set_()));
	connect(ui_.text_dropdown, SIGNAL(triggered(QAction*)), this, SLOT(insertSelectedText(QAction*)));
	connect(ui_.operation, SIGNAL(activated(int)), ui_.value, SLOT(selectAll()));
	connect(ui_.operation, SIGNAL(activated(int)), ui_.value, SLOT(setFocus()));

	//set focus
	ui_.value->selectAll();
	ui_.value->setFocus();
}

void FilterDialog::set_()
{
	Filter filter;
	filter.setValue(ui_.value->text());
	filter.setType((Filter::Type)(ui_.operation->itemData(ui_.operation->currentIndex(), Qt::UserRole).toInt()));
	column_->setFilter(filter);

	accept();
}

void FilterDialog::addOperation_(Filter::Type type)
{
	ui_.operation->addItem(Filter::typeToString(type), type);
}

void FilterDialog::updateDropdownText()
{
	if (ui_.text_dropdown->menu()->isEmpty())
	{
		//create the set of present strings (maximum 30)
		QSet<QString> set;
		bool restricted = false;
		for (int r=0; r<column_->count(); ++r)
		{
			if (set.count()<30)
			{
				set.insert(column_->string(r));
			}
			else
			{
				restricted = true;
				break;
			}
		}

		//add ordered string values to menu
		QList<QString> ordered = SET_TO_LIST(set);
		std::sort(ordered.begin(), ordered.end());
		foreach(QString value, ordered)
		{
			ui_.text_dropdown->menu()->addAction(value);
		}

		//add '...' to make clear that the list was restricted
		if (restricted)
		{
			ui_.text_dropdown->menu()->addAction("...");
		}
	}
}

void FilterDialog::insertSelectedText(QAction* action)
{
	ui_.value->insert(action->text());
}
