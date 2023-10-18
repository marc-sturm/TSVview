#include "Filter.h"
#include "CustomExceptions.h"

Filter::Filter()
	: value_("")
	, type_(NONE)
{
}

QString Filter::value() const
{
	return value_;
}

void Filter::setValue(QString value)
{
	value_ = value;
}

Filter::Type Filter::type() const
{
	return type_;
}

void Filter::setType(Filter::Type type)
{
	type_ = type;
}

QString Filter::asString(QString name, int index) const
{
	return "[" + QString::number(index) + "] '" + name + "' " + typeToString(type_) + " '" + value_ + "'";
}

QString Filter::typeToString(Filter::Type type, bool human_readable)
{
	if (human_readable)
	{
		switch(type)
		{
			case Filter::NONE:
				return "disabled";
			case Filter::STRING_EXACT:
				return "is";
			case Filter::STRING_EXACT_NOT:
				return "is not";
			case Filter::STRING_CONTAINS:
				return "contains";
			case Filter::STRING_CONTAINS_NOT:
				return "does not contain";
			case Filter::STRING_REGEXP:
				return "matches regexp";
			case Filter::STRING_REGEXP_NOT:
				return "does not match regexp";
			case Filter::FLOAT_EXACT:
				return "=";
			case Filter::FLOAT_EXACT_NOT:
				return "!=";
			case Filter::FLOAT_GREATER:
				return ">";
			case Filter::FLOAT_GREATER_EQUAL:
				return ">=";
			case Filter::FLOAT_LESS:
				return "<";
			case Filter::FLOAT_LESS_EQUAL:
				return "<=";
		}
	}
	else
	{
		switch(type)
		{
			case Filter::NONE:
				return "NONE";
			case Filter::STRING_EXACT:
				return "STRING_EXACT";
			case Filter::STRING_EXACT_NOT:
				return "STRING_EXACT_NOT";
			case Filter::STRING_CONTAINS:
				return "STRING_CONTAINS";
			case Filter::STRING_CONTAINS_NOT:
				return "STRING_CONTAINS_NOT";
			case Filter::STRING_REGEXP:
				return "STRING_REGEXP";
			case Filter::STRING_REGEXP_NOT:
				return "STRING_REGEXP_NOT";
			case Filter::FLOAT_EXACT:
				return "FLOAT_EXACT";
			case Filter::FLOAT_EXACT_NOT:
				return "FLOAT_EXACT_NOT";
			case Filter::FLOAT_GREATER:
				return "FLOAT_GREATER";
			case Filter::FLOAT_GREATER_EQUAL:
				return "FLOAT_GREATER_EQUAL";
			case Filter::FLOAT_LESS:
				return "FLOAT_LESS";
			case Filter::FLOAT_LESS_EQUAL:
				return "FLOAT_LESS_EQUAL";
		}
	}

	THROW(FilterTypeException,"Internal error: Unknown filter type!");
}

Filter::Type Filter::stringToType(QString string, bool human_readable)
{
	for(int i=0; i<13; ++i)
	{
		Type type = (Type)i;
		if(typeToString(type, human_readable)==string)
		{
			return type;
		}
	}

	THROW(FilterTypeException,"Internal error: Unknown filter type '" + string + "'!");
}

QString Filter::toString() const
{
	return "##TSVVIEW-FILTER##" + Filter::typeToString(type_, false) + "##" + value_;
}

Filter Filter::fromString(QString line)
{
	QStringList parts = line.split("##");

	if (parts.count()>3) parts[3] = parts.mid(3).join("##");

	Filter filter;
	filter.setType(Filter::stringToType(parts[2], false));
	filter.setValue(parts[3]);

	return filter;
}
