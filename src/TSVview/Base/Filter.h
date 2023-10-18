#ifndef FILTER_H
#define FILTER_H

#include <QString>

class Filter
{
public:
	enum Type
		{
		NONE,
		FLOAT_EXACT,
		FLOAT_EXACT_NOT,
		FLOAT_LESS,
		FLOAT_LESS_EQUAL,
		FLOAT_GREATER,
		FLOAT_GREATER_EQUAL,
		STRING_EXACT,
		STRING_EXACT_NOT,
		STRING_CONTAINS,
		STRING_CONTAINS_NOT,
		STRING_REGEXP,
		STRING_REGEXP_NOT
		};

	Filter();
	Filter(Type type, QString value);

	QString value() const;
	void setValue(QString value);

	Type type() const;
	void setType(Type type);

	// Returns a human-readable string representation of the filter
	QString asString(QString name, int index) const;

	static QString typeToString(Type type, bool human_readable = true);
	static Type stringToType(QString string, bool human_readable = true);

	//Parses a serialized filter
	QString toString() const;
	static Filter fromString(QString line);

protected:
	QString value_;
	Type type_;
};

#endif // FILTER_H
