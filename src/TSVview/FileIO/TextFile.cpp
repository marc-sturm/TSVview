#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QTime>
#include "TextFile.h"
#include "CustomExceptions.h"

void TextFile::load(DataSet& data, QString filename, Parameters params, int preview_lines)
{
	QFile file(filename);
	filename = QFileInfo(filename).fileName();
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		data.clear(true);
		THROW(FileIOException,"Cannot open file '" + filename + "'.");
	}

	//check which parameters to use
	if (params.size()==0)
	{
		params = defaultParameters();
	}

	QTextStream stream(&file);
	QTime timer;
	timer.start();
	determineColumns(data, stream, params, preview_lines);
	qDebug() << "determining columns c=" << data.columnCount() << "ms=" << timer.restart();

	stream.seek(0);
	fromStream(data, stream, params, filename, preview_lines);
	qDebug() << "loading data c=" << data.columnCount() << "r=" << data.rowCount() << "ms=" << timer.restart();
}

void TextFile::fromStream(DataSet& data, QTextStream& stream, QString location, Parameters params, int preview_lines)
{
	//check which parameters to use
	if (params.size()==0)
	{
		params = defaultParameters();
	}

	stream.seek(0);
	determineColumns(data, stream, params, preview_lines);

	stream.seek(0);
	fromStream(data, stream, params, location, preview_lines);
}

void TextFile::fromStream(DataSet& data, QTextStream& stream, Parameters params, QString location, int preview_lines)
{
	int cols = data.columnCount();
	int row = 0;
	QChar comment = params.getChar("comment");
	QString separator = params.getChar("separator");
	bool first_line_is_comment = params.getBool("first_line_is_comment");
	QChar quote = params.getChar("quote");
	bool has_quote = (quote!=QChar::Null);
	if (has_quote)
	{
		separator = quote + separator + quote;
	}

	QStringList comments;
	bool is_first_content_line = true;
	while (!stream.atEnd() && (preview_lines==-1 || row<preview_lines))
	{
		QString line = stream.readLine();

		// Skip empty lines
		if (line.trimmed()=="") continue;

		// Store comment lines
		if (comment!=QChar::Null && line[0]==comment)
		{
			comments.push_back(line);
			continue;
		}

		// Store forced first line comment
		if (first_line_is_comment && is_first_content_line)
		{
			is_first_content_line = false;
			if (comment!=QChar::Null && line[0]!=comment)
			{
				line = comment + line;
			}

			comments.push_back(line);
			continue;
		}

		//split
		QStringList parts = line.split(separator);
		if (parts.size()!=cols)
		{
			data.clear(true);
			THROW(FileParseException, "Wrong number of elements in line " + QString::number(row+1) + " of '" + location + "'. " +  QString::number(cols) + " elements expected, but " + QString::number(parts.count()) + " found:\n"+line);
		}

		// Handle quotes
		if (has_quote)
		{
			QString part = parts[0];
			parts[0] = part.mid(1);
			part = parts[parts.count()-1];
			parts[parts.count()-1] = part.mid(0, part.count()-1);
		}

		// Assign data
		for (int c=0; c<cols; ++c)
		{
			data.column(c).appendString(parts[c]);
		}

		++row;
	}

	// Get colmun headers from the first comment line that has the correct count
	for (int i=0; i<comments.count(); ++i)
	{
		QString comment_line = comments[i];

		//remove comment character
		if (comment!=QChar::Null)
		{
			comment_line = comment_line.mid(1);

			//skip empty and double-comment lines
			if (comment_line.trimmed().isEmpty() || comment_line[0]==comment)
			{
				continue;
			}
		}

		//split
		QStringList parts = comment_line.split(separator);

		//Use if it has the right size
		if (parts.size()==cols)
		{
			// Handle quotes
			if (has_quote)
			{
				QString part = parts[0];
				parts[0] = part.mid(1);
				part = parts[parts.count()-1];
				parts[parts.count()-1] = part.mid(0, part.count()-1);
			}

			for (int c=0; c<cols; ++c)
			{
				data.column(c).setHeader(parts[c]);
			}
			break;
		}
	}

	//update format
	for (int i=0; i<data.columnCount(); ++i)
	{
		data.column(i).autoFormat();
	}

	//store comments
	data.setComments(comments);

	data.setModified(false);
}

void TextFile::determineColumns(DataSet& data, QTextStream& stream, const Parameters& params, int preview_lines)
{
	QChar comment_char = params.getChar("comment");
	QString separator = params.getChar("separator");
	QChar quote = params.getChar("quote");
	bool has_quote = (quote!=QChar::Null);
	if (has_quote)
	{
		separator = quote + separator + quote;
	}
	bool first_line_is_comment = params.getBool("first_line_is_comment");
	QSet<int> numeric_columns;

	bool is_first_content_line = true;
	int cols = -1;
	int row = 0;
	while (!stream.atEnd() && (preview_lines==-1 || row < preview_lines))
	{
		QString line = stream.readLine();

		// Skip empty lines
		if (line.trimmed()=="") continue;

		// Skip comment lines
		if (comment_char!=QChar::Null && line[0]==comment_char) continue;

		// Skip first line if requested
		if (first_line_is_comment && is_first_content_line)
		{
			is_first_content_line = false;
			continue;
		}

		// Split
		QStringList parts = line.split(separator);
		if (cols==-1)
		{
			cols = parts.size();
			for (int i=0; i<cols; ++i) numeric_columns << i;
		}
		else
		{
			if (parts.count()!=cols)
			{
				THROW(FileParseException, "Number of columns differs for lines!\nExpected " + QString::number(cols) + ", but found " + QString::number(parts.count()) + " in line " + QString::number(row+1) + ":\n" + line);
			}
		}

		// Handle quotes
		if (has_quote)
		{
			QString part = parts[0];
			parts[0] = part.mid(1);
			part = parts[parts.count()-1];
			parts[parts.count()-1] = part.mid(0, part.count()-1);
		}

		//try to convert numbers
		foreach(int c, numeric_columns)
		{
			bool ok = true;
			parts[c].toDouble(&ok);
			if (!ok) numeric_columns.remove(c);
		}

		++row;
	}

	//set up columns
	data.clear(false);
	for (int i=0; i<cols; ++i)
	{
		if (numeric_columns.contains(i))
		{
			QVector<double> vector;
			vector.reserve(row+1);
			data.addColumn("", vector, false);
		}
		else
		{
			QVector<QString> vector;
			vector.reserve(row+1);
			data.addColumn("", vector, false);
		}
	}
}

void TextFile::store(DataSet& data, QString filename, Parameters params)
{
	// open file
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
	{
		THROW(FileIOException,"Could not open file '" + filename + "' for writing.");
	}

	// check which parameters to use
	if (params.size()==0)
	{
		params = defaultParameters();
	}

	// cache some values
	QTextStream stream(&file);
	int cols = data.columnCount();
	int rows = data.rowCount();
	QChar separator = params.getChar("separator");
	QChar comment = params.getChar("comment");
	QChar quote = params.getChar("quote");
	bool has_quote = (quote != QChar::Null);

	// write header
	if (comment!=QChar::Null)
	{
		stream << comment;
	}
	for (int col=0; col<cols; ++col)
	{
		if (col!=0)
		{
			stream << separator;
		}
		if (has_quote)
		{
			stream << quote << data.column(col).header() << quote;
		}
		else
		{
			stream << data.column(col).header();
		}
	}
	stream << "\n";

	// write data
	for (int row=0; row<rows; ++row)
	{
		if (row!=0)
		{
			stream << "\n";
		}
		for (int col=0; col<cols; ++col)
		{
			if (col!=0)
			{
				stream << separator;
			}
			if (has_quote)
			{
				stream << quote << data.column(col).string(row) << quote;
			}
			else
			{
				stream << data.column(col).string(row);
			}
		}
	}
}

Parameters TextFile::defaultParameters()
{
	Parameters params;

	QStringList valid_separators;
	valid_separators.append("\t");
	valid_separators.append(" ");
	valid_separators.append(",");
	valid_separators.append(";");
	valid_separators.append("|");
	params.addChar("separator", "", '\t', valid_separators);

	QStringList valid_comments;
	valid_comments.append("#");
	valid_comments.append(";");
	valid_comments.append("");
	params.addChar("comment", "", '#', valid_comments);

	QStringList quotes;
	quotes.append("");
	quotes.append("'");
	quotes.append("\"");
	params.addChar("quote", "", QChar::Null, quotes);
	params.addBool("first_line_is_comment", "", false);

	return params;
}
