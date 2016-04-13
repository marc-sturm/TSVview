#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QBuffer>

#include "ZXVFile.h"
#include "CustomExceptions.h"

void ZXVFile::store(DataSet& data, QString filename)
{
	// open file
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
	{
		THROW(FileIOException,"Could not open file '" + filename + "' for writing.");
	}

	// cache some values
	int rows = data.rowCount();
	int cols = data.columnCount();

	// create xml string
	QByteArray output;
	output.reserve(25 * cols * rows);

	output += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	output += QString("<XmlValues cols=\"%1\" rows=\"%2\">\n").arg(cols).arg(rows);
	for (int col=0; col<cols; ++col)
	{
		if (data.column(col).type()==BaseColumn::NUMERIC)
		{
			const NumericColumn& column = data.numericColumn(col);
			output += QString("\t<NumericColumn header=\"%1\" format=\"%2\" precision=\"%3\">\n").arg(column.header()).arg(column.format()).arg(column.decimalPlaces());

			output += QString("\t\t<Filter type=\"%1\" value=\"%2\"/>\n").arg(Filter::typeToString(column.filter().type(), false), column.filter().value());

			for (int row=0; row<rows; ++row)
			{
				output += QString("\t\t<Cell value=\"%1\" />\n").arg(column.value(row), 0, 'f');
			}

			output.append("\t</NumericColumn>\n");
		}
		else
		{
			const StringColumn& column = data.stringColumn(col);
			output += QString("\t<StringColumn header=\"%1\">\n").arg(column.header());

			output += QString("\t\t<Filter type=\"%1\" value=\"%2\"/>\n").arg(Filter::typeToString(column.filter().type(), false), column.filter().value());

			for (int row=0; row<rows; ++row)
			{
				output += QString("\t\t<Cell value=\"%1\" />\n").arg(column.value(row));
			}

			output.append("\t</StringColumn>\n");
		}
	}
	output.append("</XmlValues>\n");

	// compress xml string
	output = qCompress(output);

	// write contents to file
	QDataStream stream(&file);
	stream.writeBytes(output, output.size());
	file.close();
}

void ZXVFile::load(DataSet& data, QString filename)
{
	data.clear(false);

	ZXVParser().parse(data, filename);

	//update format
	for (int i=0; i<data.columnCount(); ++i)
	{
		data.column(i).autoFormat();
	}

	data.setModified(false);
}

ZXVFile::ZXVParser::ZXVParser()
	: QXmlDefaultHandler()
	, data_(0)
	, column_(-1)
	, row_(-1)
	, error_string_("")
{
}

void ZXVFile::ZXVParser::parse(DataSet& data, QString filename)
{
	//reset parser
	data_ = &data;
	column_ = -1;
	row_ = -1;
	error_string_ = "";

	//open file
	QFile file(filename);
	filename = QFileInfo(filename).fileName();
	if (!file.open(QIODevice::ReadOnly))
	{
		data.clear(true);
		THROW(FileIOException,"Cannot open file '" + filename + "'.");
	}

	//load data
	QByteArray compressed;
	QDataStream inputStream(&file);
	inputStream >> compressed;
	QByteArray xml_string = qUncompress(compressed);

	// fill buffer with uncompressed data
	QBuffer buffer;
	buffer.setData(xml_string);

	//set up parser
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(this);
	xmlReader.setErrorHandler(this);

	//parse file
	bool ok = xmlReader.parse(new QXmlInputSource(&buffer));
	if (!ok)
	{
		data.clear(true);
		THROW(FileIOException,"Cannot parse XML file '" + filename + "': " + error_string_);
	}
}

bool ZXVFile::ZXVParser::startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attrs )
{
	if (name=="XmlValues")
	{
		row_count_ = attrs.value("rows").toInt();
	}
	else if (name=="NumericColumn")
	{
		row_ = -1;
		++column_;

		data_->addColumn(attrs.value("header"), QVector<double>(row_count_), false);
		data_->numericColumn(column_).setFormat(attrs.value("format").at(0).toLatin1(), attrs.value("precision").toInt());
	}
	else if (name=="StringColumn")
	{
		row_ = -1;
		++column_;

		data_->addColumn(attrs.value("header"), QVector<QString>(row_count_), false);
	}
	else if (name=="Filter")
	{
		Filter filter;
		filter.setType(Filter::stringToType(attrs.value("type"), false));
		filter.setValue(attrs.value("value"));
		data_->column(column_).setFilter(filter);
	}
	else if (name=="Cell")
	{
		++row_;

		try
		{
			QString value = attrs.value("value");
			data_->column(column_).setString(row_, value);
		}
		catch (Exception&)
		{
			error_string_ = "Cannot convert data attribute '" + attrs.value("value") + "' to a number.";
			return false;
		}
	}

	return true;
}

bool ZXVFile::ZXVParser::error(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

bool ZXVFile::ZXVParser::fatalError(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

QString ZXVFile::ZXVParser::errorString()
{
	return "Dummy";
}
