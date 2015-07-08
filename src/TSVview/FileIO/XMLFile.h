#ifndef XMLFILE_H
#define XMLFILE_H

#include <QtXml/QXmlDefaultHandler>

#include "DataSet.h"

class XMLFile
{
public:
	static void load(DataSet& data, QString filename, Parameters params);
	static void store(DataSet& data, QString filename, Parameters params);

	static Parameters defaultParameters();

private:
	XMLFile();

	class RowWiseXMLParser
			: public QXmlDefaultHandler
	{
	public:
		RowWiseXMLParser();
		void parse(DataSet& data, QString filename, QString row_tag, QString data_tag, QString data_attribute = QString::null);

	protected:
		virtual bool startElement(const QString&, const QString&, const QString& name, const QXmlAttributes& attrs);
		virtual bool endElement(const QString&, const QString&, const QString& name );
		virtual bool characters(const QString& ch);
		virtual bool error(const QXmlParseException& exception);
		virtual bool fatalError(const QXmlParseException& exception);

		bool convertToDouble_(QString string);

	private:
		DataSet* data_;
		QString main_tag_;
		QString data_tag_;
		QString data_attribute_;
		bool in_main_tag_;
		bool in_data_tag_;
		int column_;
		int row_;
		QString error_string_;
	};

	class ColumnWiseXMLParser
			: public QXmlDefaultHandler
	{
	public:
		ColumnWiseXMLParser();
		void parse(DataSet& data, QString filename, QString column_tag, QString data_tag, QString data_attribute = QString::null);

	protected:
		virtual bool startElement(const QString&, const QString&, const QString& name, const QXmlAttributes& attrs);
		virtual bool endElement(const QString&, const QString&, const QString& name );
		virtual bool characters(const QString& ch);
		virtual bool error(const QXmlParseException& exception);
		virtual bool fatalError(const QXmlParseException& exception);

		bool convertToDouble_(QString string);

	private:
		DataSet* data_;
		QString main_tag_;
		QString data_tag_;
		QString data_attribute_;
		bool in_main_tag_;
		bool in_data_tag_;
		int column_;
		int row_;
		QString error_string_;
	};

};

#endif
