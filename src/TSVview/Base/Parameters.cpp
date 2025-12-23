#include "Parameters.h"
#include "CustomExceptions.h"
#include <QPoint>
#include <QRegularExpression>

Parameters::Parameters(QObject* object)
	: QObject(object)
{
}

Parameters::Parameters(const Parameters& rhs)
	: QObject(0)
	, params_(rhs.params_)
{
}

Parameters& Parameters::operator=(const Parameters& rhs)
{
	params_ = rhs.params_;

	return *this;
}

void Parameters::addInt(QString key, QString description, int value, int min, int max)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = Int;
	data.v_variant = value;
	data.description = description;
	if (min!=-std::numeric_limits<int>::max() || max!=std::numeric_limits<int>::max())
	{
		data.restriction = QPoint(min, max);
	}
	insert_(data);
}

void Parameters::addDouble(QString key, QString description, double value, double min, double max)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = Double;
	data.v_variant = value;
	data.description = description;
	if (min!=-std::numeric_limits<double>::max() || max!=std::numeric_limits<double>::max())
	{
		data.restriction = QPointF(min, max);
	}
	insert_(data);
}

void Parameters::addString(QString key, QString description, QString value, QStringList valid)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = String;
	data.v_variant = value;
	data.description = description;
	if (!valid.empty())
	{
		data.restriction = valid;
	}
	insert_(data);
}

void Parameters::addChar(QString key, QString description, QChar value, QStringList valid)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = Char;
	data.v_variant = value;
	data.description = description;
	if (!valid.empty())
	{
		data.restriction = valid;
	}
	insert_(data);
}

void Parameters::addBool(QString key, QString description, bool value)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = Bool;
	data.v_variant = value;
	data.description = description;
	insert_(data);
}

void Parameters::addColor(QString key, QString description, QColor value)
{
	checkKey_(key);
	checkExists_(key, false);

	Data_ data(key);
	data.type = Color;
	data.v_variant = value;
	data.description = description;
	insert_(data);
}

void Parameters::addSeparator()
{
	if (params_.size()==0) return;

	params_.last().separator = true;
}

void Parameters::setInt(QString key, int value)
{
	checkIsType_(key, Int);

	if (isRestricted_(key))
	{
		QPoint bounds = get_(key).restriction.toPoint();
		if (value < bounds.x() || value > bounds.y())
		{
			THROW(ParameterException, "Value '" + QString::number(value) + "' is not valid for restricted integer parameter '" + key + "'.");
		}
	}

	get_(key).v_variant = value;
	emit valueChanged(key);
}

void Parameters::setDouble(QString key, double value)
{
	checkIsType_(key, Double);

	if (isRestricted_(key))
	{
		QPointF bounds = get_(key).restriction.toPointF();
		if (value < bounds.x() || value > bounds.y())
		{
			THROW(ParameterException, "Value '" + QString::number(value) + "' is not valid for restricted double parameter '" + key + "'.");
		}
	}

	get_(key).v_variant = value;
	emit valueChanged(key);
}

void Parameters::setString(QString key, QString value)
{
	checkIsType_(key, String);

	if (isRestricted_(key) && !get_(key).restriction.toStringList().contains(value))
	{
		THROW(ParameterException, "Value '" + value + "' is not valid for restricted string parameter '" + key + "'.");
	}

	get_(key).v_variant = value;
	emit valueChanged(key);
}

void Parameters::setChar(QString key, QChar value)
{
	checkIsType_(key, Char);

	if (isRestricted_(key))
	{
		bool invalid = false;
		if (value==QChar::Null && !get_(key).restriction.toStringList().contains(""))
		{
			invalid = true;
		}
		if (value!=QChar::Null && !get_(key).restriction.toStringList().contains(value))
		{
			invalid = true;
		}

		if (invalid)
		{
			THROW(ParameterException, "Value '" + QString(value) + "' is not valid for restricted character parameter '" + key + "'.");
		}
	}

	get_(key).v_variant = value;
	emit valueChanged(key);
}

void Parameters::setBool(QString key, bool value)
{
	checkIsType_(key, Bool);

	get_(key).v_variant = value;
	emit valueChanged(key);
}

void Parameters::setColor(QString key, QColor value)
{
	checkIsType_(key, Color);

	get_(key).v_variant = value;
	emit valueChanged(key);
}

int Parameters::getInt(QString key) const
{
	checkIsType_(key, Int);

	return get_(key).v_variant.toInt();
}

QString Parameters::getString(QString key) const
{
	checkIsType_(key, String);

	return get_(key).v_variant.toString();
}

QChar Parameters::getChar(QString key) const
{
	checkIsType_(key, Char);

	return get_(key).v_variant.toChar();
}

double Parameters::getDouble(QString key) const
{
	checkIsType_(key, Double);

	return get_(key).v_variant.toDouble();
}

bool Parameters::getBool(QString key) const
{
	checkIsType_(key, Bool);

	return get_(key).v_variant.toBool();
}

QColor Parameters::getColor(QString key, int alpha) const
{
	checkIsType_(key, Color);

	QColor color = get_(key).v_variant.value<QColor>();
	color.setAlpha(alpha);
	return color;
}

Parameters::Type Parameters::type(QString key) const
{
	checkExists_(key);

	return get_(key).type;
}

QStringList Parameters::keys() const
{
	QStringList output;
	output.reserve(params_.size());
	for (int i=0;i<params_.count(); ++i)
	{
		output.append(params_[i].name);
	}
	return output;
}

QStringList Parameters::sections() const
{
	QStringList output;
	foreach(QString key, keys())
	{
		int index = key.indexOf(':');
		if (index!=-1)
		{
			QString section = key.left(index);
			if (!output.contains(section))
			{
				output.append(section);
			}
		}
	}

	return output;
}

void Parameters::clear()
{
	params_.clear();
}

void Parameters::clearRestrictions()
{
	for (int i=0;i<params_.count(); ++i)
	{
		params_[i].restriction.clear();
	}
}

bool Parameters::isRestricted(QString key) const
{
	checkExists_(key);

	return isRestricted_(key);
}

bool Parameters::isRestricted_(QString key) const
{
	return get_(key).restriction.isValid();
}

Parameters::Data_& Parameters::get_(QString key)
{
	for (int i=0;i<params_.count(); ++i)
	{
		if (params_[i].name==key)
		{
			return params_[i];
		}
	}

	THROW(ParameterException, "Could not get_ parameter with name " + key + ". This should not happen!");
}

const Parameters::Data_ &Parameters::get_(QString key) const
{
	for (int i=0;i<params_.count(); ++i)
	{
		if (params_[i].name==key)
		{
			return params_[i];
		}
	}

	THROW(ParameterException, "Could not get_ parameter with name " + key + ". This should not happen!");
}

void Parameters::insert_(const Parameters::Data_& data)
{
	params_.append(data);
}

bool Parameters::contains_(QString key) const
{
	for (int i=0;i<params_.count(); ++i)
	{
		if (params_[i].name==key)
		{
			return true;
		}
	}
	return false;
}

void Parameters::getIntBounds(QString key, int& min, int& max)
{
	checkIsType_(key, Int);

	if (!isRestricted_(key))
	{
		THROW(ParameterException, "The integer parameter '" + key + "' is not restricted.");
	}

	QPoint bounds = get_(key).restriction.toPoint();
	min = bounds.x();
	max = bounds.y();
}

void Parameters::getDoubleBounds(QString key, double& min, double& max)
{
	checkIsType_(key, Double);

	if (!isRestricted_(key))
	{
		THROW(ParameterException, "The double parameter '" + key + "' is not restricted.");
	}

	QPointF bounds = get_(key).restriction.toPointF();
	min = bounds.x();
	max = bounds.y();
}

QStringList Parameters::getValidStrings(QString key)
{
	checkIsType_(key, String);

	if (!isRestricted_(key))
	{
		THROW(ParameterException, "The string parameter '" + key + "' is not restricted.");
	}

	return get_(key).restriction.toStringList();
}

QStringList Parameters::getValidCharacters(QString key)
{
	checkIsType_(key, Char);

	if (!isRestricted_(key))
	{
		THROW(ParameterException, "The character parameter '" + key + "' is not restricted.");
	}

	return get_(key).restriction.toStringList();
}

bool Parameters::separatorAfter(QString key)
{
	return get_(key).separator;
}

QString Parameters::description(QString key)
{
	return get_(key).description;
}

int Parameters::size() const
{
	return params_.size();
}

QString Parameters::typeToString_(Parameters::Type type)
{
	switch(type)
	{
		case Int:
			return "int";
			break;
		case Double:
			return "double";
			break;
		case String:
			return "string";
			break;
		case Char:
			return "char";
			break;
		case Bool:
			return "bool";
			break;
		case Color:
			return "color";
			break;
	}

	//never reached, but we required to suppress the warning...
	return "None";
}

void Parameters::checkExists_(QString key, bool exists) const
{
	if (exists && !contains_(key))
	{
		THROW(ParameterException, "Parameter '" + key + "' does not exist.");
	}
	else if (!exists && contains_(key))
	{
		THROW(ParameterException, "Parameter '" + key + "' already exist.");
	}
}

void Parameters::checkIsType_(QString key, Parameters::Type type) const
{
	checkExists_(key);

	if (get_(key).type!=type)
	{
		THROW(ParameterException, "Unable to access parameter '" + key + "' as '" + typeToString_(type) + "'. It is of type '" + typeToString_(get_(key).type) + "'.");
	}
}

void Parameters::checkKey_(QString key)
{
	if (key.size()==0)
	{
		THROW(ParameterException, "Invalid key '" + key + "'. It must not be empty!");
	}
	else if (key.startsWith(':') || key.endsWith(':') || key.count(':')>1)
	{
		THROW(ParameterException, "Invalid key '" + key + "'. Invalid use of section separator ':'.");
	}
	else if (!QRegularExpression("[A-Za-z0-9_: -\\[\\]]+").match(key).hasMatch())
	{
		THROW(ParameterException, "Invalid key '" + key + "'. Invalid character used.");
	}
}
