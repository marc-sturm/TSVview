#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QColor>
#include <limits>

class Parameters
		: public QObject
{
	Q_OBJECT

public:
	enum Type
		{
		Int,
		Double,
		String,
		Char,
		Bool,
		Color
		};

	Parameters(QObject* object=0);
	Parameters(const Parameters& rhs);
	Parameters& operator=(const Parameters& rhs);

	void addInt(QString key, QString description, int value, int min = -std::numeric_limits<int>::max(), int max = std::numeric_limits<int>::max());
	void addDouble(QString key, QString description, double value, double min = -std::numeric_limits<double>::max(), double max = std::numeric_limits<double>::max());
	void addString(QString key, QString description, QString value, QStringList valid = QStringList());
	void addChar(QString key, QString description, QChar value, QStringList valid = QStringList());
	void addBool(QString key, QString description, bool value);
	void addColor(QString key, QString description, QColor value);
	void addSeparator();

	void setInt(QString key, int value);
	void setDouble(QString key, double value);
	void setString(QString key, QString value);
	void setChar(QString key, QChar value);
	void setBool(QString key, bool value);
	void setColor(QString key, QColor value);

	int getInt(QString key) const;
	double getDouble(QString key) const;
	QString getString(QString key) const;
	QChar getChar(QString key) const;
	bool getBool(QString key) const;
	QColor getColor(QString key, int alpha=255) const;

	Type type(QString key) const;
	bool isRestricted(QString key) const;
	void getIntBounds(QString key, int& min, int& max);
	void getDoubleBounds(QString key, double& min, double& max);
	QStringList getValidStrings(QString key);
	QStringList getValidCharacters(QString key);
	bool separatorAfter(QString key);
	QString description(QString key);

	int size() const;
	QStringList keys() const;
	QStringList sections() const;
	void clear();
	void clearRestrictions();

signals:
	void valueChanged(QString);

private:
	struct Data_
	{
		Data_(QString name)
			: name(name)
			, description("")
			, type(Int)
			, v_variant()
			, restriction(QVariant::Invalid)
			, separator(false)
		{
		}

		QString name;
		QString description;
		Type type;
		QVariant v_variant;
		QVariant restriction;
		bool separator;
	};

	QList<Data_> params_;

	static QString typeToString_(Type type);
	void checkExists_(QString key, bool exists = true) const;
	void checkIsType_(QString key, Parameters::Type type) const;
	static void checkKey_(QString key);
	bool isRestricted_(QString key) const;
	Data_& get_(QString key);
	const Data_& get_(QString key) const;
	void insert_(const Data_& data);
	bool contains_(QString key) const;
};

#endif
