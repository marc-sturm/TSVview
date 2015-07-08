#ifndef ZXVFile_H
#define ZXVFile_H

#include <QXmlDefaultHandler>

#include "DataSet.h"

class ZXVFile
{
public:
	static void load(DataSet& data, QString filename);
	static void store(DataSet& data, QString filename);

private:
	ZXVFile();

	class ZXVParser
			: public QXmlDefaultHandler
	{
	public:
		ZXVParser();
		void parse(DataSet& data, QString filename);

	protected:
		virtual bool startElement(const QString&, const QString&, const QString& name, const QXmlAttributes& attrs);
		virtual bool error(const QXmlParseException& exception);
		virtual bool fatalError(const QXmlParseException& exception);

	private:
		DataSet* data_;
		int column_;
		int row_;
		int row_count_;
		QString error_string_;
	};
};

#endif
