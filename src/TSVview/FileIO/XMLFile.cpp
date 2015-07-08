#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>

#include "XMLFile.h"
#include "CustomExceptions.h"

void XMLFile::load(DataSet& data, QString filename, Parameters params)
{
	data.clear(false);

	QString orientation = params.getString("orientation");
	QString main_tag = params.getString("main tag");
	QString data_tag = params.getString("data tag");
	QString data_attribute = params.getString("data attribute");
	if (data_attribute.trimmed()=="")
	{
		data_attribute = QString::null;
	}

	if (orientation=="row")
	{
		RowWiseXMLParser().parse(data, filename, main_tag, data_tag, data_attribute);
	}
	else if (orientation=="column")
	{
		ColumnWiseXMLParser().parse(data, filename, main_tag, data_tag, data_attribute);
	}
	else
	{
		THROW(FileIOException,"Invalid orientation '" + orientation + "' for xml import of file '" + filename + "'.");
	}

	//update format
	for (int i=0; i<data.columnCount(); ++i)
	{
		data.column(i).autoFormat();
	}

	data.setModified(false);
}

void XMLFile::store(DataSet& data, QString filename, Parameters params)
{
	// open file
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
	{
		THROW(FileIOException,"Could not open file '" + filename + "' for writing.");
	}

	// cache some values
	QTextStream stream(&file);
	int cols = data.columnCount();
	int rows = data.rowCount();
	QString main_tag = params.getString("main tag").trimmed();
	QString data_tag = params.getString("data tag").trimmed();
	QString data_attribute = params.getString("data attribute").trimmed();
	QString orientation = params.getString("orientation");

	if ((orientation!="row" && orientation!="column") || main_tag=="" || data_tag=="")
	{
		THROW(FileIOException,"Invalid parameters for XML file export of '" + filename + "'.");
	}

	// write header
	stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";

	// row-wise
	stream << "<XmlExport>\n";
	if (orientation=="row")
	{
		for (int row=0; row<rows; ++row)
		{
			stream << "\t<" << main_tag << ">\n";
			for (int col=0; col<cols; ++col)
			{
				if (data_attribute=="")
				{
					stream << "\t\t<" << data_tag << ">" << data.column(col).string(row) << "</" << data_tag << ">\n";
				}
				else
				{
					stream << "\t\t<" << data_tag << " " << data_attribute << "=\"" << data.column(col).string(row) << "\" />\n";
				}
			}
			stream << "\t</" << main_tag << ">\n";
		}
	}
	// column-wise
	else if (orientation=="column")
	{
		for (int col=0; col<cols; ++col)
		{
			stream << "\t<" << main_tag << ">\n";
			const BaseColumn& column = data.column(col);
			for (int row=0; row<rows; ++row)
			{
				if (data_attribute=="")
				{
					stream << "\t\t<" << data_tag << ">" << column.string(row) << "</" << data_tag << ">\n";
				}
				else
				{
					stream << "\t\t<" << data_tag << " " << data_attribute << "=\"" << column.string(row) << "\" />\n";
				}
			}
			stream << "\t</" << main_tag << ">\n";
		}
	}
	stream << "</XmlExport>\n";
}

Parameters XMLFile::defaultParameters()
{
	Parameters params;

	params.addString("main tag", "", "");
	params.addString("data tag", "", "");
	params.addString("data attribute", "", "");
	QStringList valid_orientations;
	valid_orientations.append("row");
	valid_orientations.append("column");
	params.addString("orientation", "", "column", valid_orientations);

	return params;
}

XMLFile::RowWiseXMLParser::RowWiseXMLParser()
	: QXmlDefaultHandler()
	, data_(0)
	, main_tag_("")
	, data_tag_("")
	, data_attribute_("")
	, in_main_tag_(false)
	, column_(-1)
	, row_(-1)
	, error_string_("")
{
}

void XMLFile::RowWiseXMLParser::parse(DataSet& data, QString filename, QString row_tag, QString data_tag, QString data_attribute)
{
	//reset parser
	data_ = &data;
	main_tag_ = row_tag;
	data_tag_ = data_tag;
	data_attribute_ = data_attribute;
	in_main_tag_ = false;
	in_data_tag_ = false;
	column_ = -1;
	row_ = -1;
	error_string_ = "";

	//set up parser
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(this);
	xmlReader.setErrorHandler(this);

	//parse
	QFile file(filename);
	filename = QFileInfo(filename).fileName();
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		data.clear(true);
		THROW(FileIOException,"Cannot open file '" + filename + "'.");
	}

	QXmlInputSource* source = new QXmlInputSource(&file);
	bool ok = xmlReader.parse(source);
	if (!ok)
	{
		data.clear(true);
		THROW(FileIOException,"Cannot parse XML file '" + filename + "': " + error_string_);
	}
}

bool XMLFile::RowWiseXMLParser::startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attrs )
{
	if(in_main_tag_ && name==data_tag_)
	{
		++column_;

		if (column_>=data_->columnCount())
		{
			if (data_->columnCount()==0 || data_->rowCount()==1)
			{
				data_->addColumn("", QVector<double>(1), false);
			}
			else
			{
				error_string_ = "Wrong number of data tags encountered. " + QString::number(data_->columnCount()) + " data tags expected.";
				return false;
			}
		}

		if (data_attribute_!=QString::null)
		{
			int index = attrs.index(data_attribute_);
			if(index==-1)
			{
				error_string_ = "Data tag attribute not found.";
				return false;
			}

			if(!convertToDouble_(attrs.value(index)))
			{
				error_string_ = "Cannot convert data attribute '" + attrs.value(index) + "' to a number.";
				return false;
			}
		}
		else
		{
			in_data_tag_ = true;
		}
	}
	else if (name==main_tag_)
	{
		in_main_tag_ = true;
		++row_;
		column_ = -1;

		for (int c=0; c<data_->columnCount(); ++c)
		{
			data_->column(c).resize(row_+1);
		}
	}

	return true;
}

bool XMLFile::RowWiseXMLParser::endElement(const QString&, const QString&, const QString& name )
{
	if (in_main_tag_ && name==main_tag_)
	{
		in_main_tag_ = false;
	}
	if (in_data_tag_ && name==data_tag_)
	{
		in_data_tag_ = false;
	}

	return true;
}

bool XMLFile::RowWiseXMLParser::characters(const QString& ch)
{
	if (in_data_tag_)
	{
		QString characters = ch.trimmed();
		if (characters!="")
		{
			if(!convertToDouble_(characters))
			{
				error_string_ = "Cannot convert data tag CDATA '" + characters + "' to a number.";
				return false;
			}
		}
	}

	return true;
}

bool XMLFile::RowWiseXMLParser::error(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

bool XMLFile::RowWiseXMLParser::fatalError(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

bool XMLFile::RowWiseXMLParser::convertToDouble_(QString characters)
{
	bool ok;
	characters.toDouble(&ok);

	if (ok)
	{
		data_->column(column_).setString(row_, characters);
	}

	return ok;
}

XMLFile::ColumnWiseXMLParser::ColumnWiseXMLParser()
	: QXmlDefaultHandler()
	, data_(0)
	, main_tag_("")
	, data_tag_("")
	, data_attribute_("")
	, in_main_tag_(false)
	, column_(-1)
	, row_(-1)
	, error_string_("")
{
}

void XMLFile::ColumnWiseXMLParser::parse(DataSet& data, QString filename, QString row_tag, QString data_tag, QString data_attribute)
{
	//reset parser
	data_ = &data;
	main_tag_ = row_tag;
	data_tag_ = data_tag;
	data_attribute_ = data_attribute;
	in_main_tag_ = false;
	in_data_tag_ = false;
	column_ = -1;
	row_ = -1;
	error_string_ = "";

	//set up parser
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(this);
	xmlReader.setErrorHandler(this);

	//parse
	QFile file(filename);
	filename = QFileInfo(filename).fileName();
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		data.clear(true);
		THROW(FileIOException,"Cannot open file '" + filename + "'.");
	}

	QXmlInputSource* source = new QXmlInputSource(&file);
	bool ok = xmlReader.parse(source);
	if (!ok)
	{
		data.clear(true);
		THROW(FileIOException,"Cannot parse XML file '" + filename + "': " + error_string_);
	}
}

bool XMLFile::ColumnWiseXMLParser::startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attrs )
{
	if(in_main_tag_ && name==data_tag_)
	{
		++row_;

		if (row_>=data_->rowCount())
		{
			if (column_==0)
			{
				for (int c=0; c<data_->columnCount(); ++c)
				{
					data_->column(c).resize(row_+1);
				}
			}
			else
			{
				error_string_ = "Wrong number of data tags encountered. " + QString::number(data_->rowCount()) + " data tags expected.";
				return false;
			}
		}

		if (data_attribute_!=QString::null)
		{
			int index = attrs.index(data_attribute_);
			if(index==-1)
			{
				error_string_ = "Data tag attribute not found.";
				return false;
			}

			if(!convertToDouble_(attrs.value(index)))
			{
				error_string_ = "Cannot convert data attribute '" + attrs.value(index) + "' to a number.";
				return false;
			}
		}
		else
		{
			in_data_tag_ = true;
		}
	}
	else if (name==main_tag_)
	{
		++column_;

		in_main_tag_ = true;
		row_ = -1;
		data_->addColumn("", QVector<double>(data_->rowCount()), false);
	}

	return true;
}

bool XMLFile::ColumnWiseXMLParser::endElement(const QString&, const QString&, const QString& name )
{
	if (in_main_tag_ && name==main_tag_)
	{
		in_main_tag_ = false;
	}
	if (in_data_tag_ && name==data_tag_)
	{
		in_data_tag_ = false;
	}

	return true;
}

bool XMLFile::ColumnWiseXMLParser::characters(const QString& ch)
{
	if (in_data_tag_)
	{
		QString characters = ch.trimmed();
		if (characters!="")
		{
			if(!convertToDouble_(characters))
			{
				error_string_ = "Cannot convert data tag CDATA '" + characters + "' to a number.";
				return false;
			}
		}
	}

	return true;
}

bool XMLFile::ColumnWiseXMLParser::error(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

bool XMLFile::ColumnWiseXMLParser::fatalError(const QXmlParseException& exception)
{
	error_string_ = exception.message();

	return false;
}

bool XMLFile::ColumnWiseXMLParser::convertToDouble_(QString characters)
{
	try
	{
		data_->column(column_).setString(row_, characters);
	}
	catch (Exception& e)
	{
		return false;
	}

	return true;
}

