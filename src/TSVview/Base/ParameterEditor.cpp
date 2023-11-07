#include <QDialog>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPainter>
#include <QDialogButtonBox>

#include "ParameterEditor.h"
#include "CustomExceptions.h"
#include "ColorSelector.h"
#include "GUIHelper.h"

ParameterEditor::ParameterEditor(QWidget* parent)
	: QWidget(parent)
	, combo_(0)
	, stack_(0)
	, params_(0)
{
	//create layout
	setLayout(new QBoxLayout(QBoxLayout::TopToBottom));
	setMaximumSize(300, 2000);  //workaround for QDoubleSpinBox bug
	layout()->setMargin(2);
}

void ParameterEditor::setParameters(Parameters& parameters)
{
	params_ = &parameters;

	setupUi();
}

void ParameterEditor::setupUi()
{
	//delete all children
	for (int i=0; i<layout()->children().count(); ++i)
	{
		delete layout()->takeAt(0);
	}

	//set up combo box with sections
	QStringList sections = params_->sections();
	bool use_sections = sections.count();
	if (use_sections)
	{
		combo_ = new QComboBox(this);
		combo_->addItems(sections);
		layout()->addWidget(combo_);
	}
	else
	{
		combo_ = 0;
	}

	//add stack widget
	stack_ = new QStackedWidget(this);
	layout()->addWidget(stack_);
	if (use_sections)
	{
		connect(combo_, SIGNAL(activated(int)), stack_, SLOT(setCurrentIndex(int)));
	}

	//add stack pages
	int pages = std::max(1, sections.size());
	for (int i=0; i<pages; ++i)
	{
		QWidget* w = new QWidget(stack_);
		w->setLayout(new QFormLayout());
		w->layout()->setMargin(0);
		stack_->addWidget(w);
	}

	//set page content
	if (use_sections)
	{
		foreach(QString key, params_->keys())
		{
			int index = key.indexOf(':');
			QString section = key.left(index);
			QString name = key.right(key.size()-index-1);

			QWidget* page = stack_->widget(sections.indexOf(section));
			QFormLayout* layout = qobject_cast<QFormLayout*>(page->layout());
			QLabel* label = new QLabel(name + ":");
			label->setToolTip(params_->description(key));
			layout->addRow(label, createWidget_(key, page));
			if (params_->separatorAfter(key))
			{
				layout->addRow(GUIHelper::horizontalLine());
			}
		}
	}
	else
	{
		QWidget* page = stack_->widget(0);
		QFormLayout* layout = qobject_cast<QFormLayout*>(page->layout());
		foreach(QString key, params_->keys())
		{
			QLabel* label = new QLabel(key + ":");
			label->setToolTip(params_->description(key));
			layout->addRow(label, createWidget_(key, page));
			if (params_->separatorAfter(key))
			{
				layout->addRow(GUIHelper::horizontalLine());
			}
		}
	}
}

QWidget* ParameterEditor::createWidget_(QString key, QWidget* parent)
{
	Parameters::Type type = params_->type(key);
	switch(type)
	{
		case Parameters::Int:
		{
			QSpinBox* widget = new QSpinBox(parent);
			widget->setObjectName(key);
			widget->setValue(params_->getInt(key));
			if (params_->isRestricted(key))
			{
				int min, max;
				params_->getIntBounds(key, min, max);
				widget->setRange(min, max);
			}
			else
			{
				widget->setRange(-std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
			}
			connect(widget, SIGNAL(valueChanged(int)), this, SLOT(change_(int)));
			return widget;
		}
			break;
		case Parameters::Color:
		{
			ColorSelector* widget = new ColorSelector(parent);
			widget->setObjectName(key);
			widget->setColor(params_->getColor(key));
			connect(widget, SIGNAL(valueChanged(QColor)), this, SLOT(change_(QColor)));
			return widget;
		}
			break;
		case Parameters::Double:
		{
			QDoubleSpinBox* widget = new QDoubleSpinBox(parent);
			widget->setObjectName(key);
			widget->setValue(params_->getDouble(key));
			widget->setSingleStep(0.01);
			if (params_->isRestricted(key))
			{
				double min, max;
				params_->getDoubleBounds(key, min, max);
				widget->setRange(min, max);
			}
			else
			{
				widget->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
			}
			connect(widget, SIGNAL(valueChanged(double)), this, SLOT(change_(double)));
			return widget;
		}
			break;
		case Parameters::String:
		{
			if (params_->isRestricted(key))
			{
				QStringList validStrings = params_->getValidStrings(key);

				QComboBox* widget = new QComboBox(parent);
				widget->setObjectName(key);
				QString value = params_->getString(key);
				for (int i=0; i<validStrings.count(); ++i)
				{
					QString validString = validStrings.at(i);
					widget->addItem(validString);
					if (validString==value)
					{
						widget->setCurrentIndex(i);
					}
				}
				connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(changeString_(int)));
				return widget;
			}
			else
			{
				QLineEdit* widget = new QLineEdit(parent);
				widget->setObjectName(key);
				widget->setText(params_->getString(key));
				connect(widget, SIGNAL(textChanged(const QString&)), this, SLOT(change_(const QString&)));
				return widget;
			}
		}
			break;
		case Parameters::Char:
		{
			if (params_->isRestricted(key))
			{
				QStringList validChars = params_->getValidCharacters(key);

				QComboBox* widget = new QComboBox(parent);
				widget->setObjectName(key);
				QString value = params_->getChar(key);
				for (int i=0; i<validChars.count(); ++i)
				{
					QString validChar = validChars[i];
					if (validChar=="\t")
					{
						widget->addItem("[tab]");
					}
					else if (validChar==" ")
					{
						widget->addItem("[space]");
					}
					else if (validChar=="")
					{
						widget->addItem("[null]");
					}
					else
					{
						widget->addItem(validChar);
					}
					if (validChar==value)
					{
						widget->setCurrentIndex(i);
					}
				}
				connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(changeChar_(int)));
				return widget;
			}
			else
			{
				QLineEdit* widget = new QLineEdit(parent);
				widget->setObjectName(key);
				widget->setText(params_->getChar(key));
				widget->setMaxLength(1);
				connect(widget, SIGNAL(textChanged(const QString&)), this, SLOT(changeChar_(const QString&)));
				return widget;
			}
		}
			break;
		case Parameters::Bool:
		{
			QCheckBox* widget = new QCheckBox(parent);
			widget->setObjectName(key);
			widget->setChecked(params_->getBool(key));
			connect(widget, SIGNAL(toggled(bool)), this, SLOT(change_(bool)));
			return widget;
		}
			break;
	}

	return 0;
}

void ParameterEditor::change_(int value)
{
	params_->setInt(sender()->objectName(), value);
}

void ParameterEditor::change_(double value)
{
	params_->setDouble(sender()->objectName(), value);
}

void ParameterEditor::change_(const QString& value)
{
	params_->setString(sender()->objectName(), value);
}

void ParameterEditor::change_(bool value)
{
	params_->setBool(sender()->objectName(), value);
}

void ParameterEditor::change_(QColor value)
{
	params_->setColor(sender()->objectName(), value);
}

void ParameterEditor::changeString_(int value)
{
	QString key = sender()->objectName();
	params_->setString(key, params_->getValidStrings(key).at(value));
}

void ParameterEditor::changeChar_(int value)
{
	QString key = sender()->objectName();
	QString valid = params_->getValidCharacters(key).at(value);
	if (valid.size()==1)
	{
		params_->setChar(key, valid.at(0));
	}
	else
	{
		params_->setChar(key, QChar::Null);
	}
}

void ParameterEditor::changeChar_(const QString& value)
{
	if (value.size()==0)
	{
		params_->setChar(sender()->objectName(), QChar::Null);
	}
	else
	{
		params_->setChar(sender()->objectName(), value.at(0));
	}
}

bool ParameterEditor::asDialog(QIcon icon, QString window_title, Parameters& parameters)
{
	ParameterEditor* editor = new ParameterEditor();
	editor->setParameters(parameters);
	auto dlg = GUIHelper::createDialog(editor, window_title, "", true);
	dlg->setWindowIcon(icon);
	return dlg->exec()==QDialog::Accepted;
}

