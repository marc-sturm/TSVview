#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QElapsedTimer>
#include <QMessageBox>
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
	QElapsedTimer timer;
	timer.start();
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

	QElapsedTimer timer;
	timer.start();
	fromStream(data, stream, params, location, preview_lines);
	qDebug() << "loading data c=" << data.columnCount() << "r=" << data.rowCount() << "ms=" << timer.restart();
}

void TextFile::fromStream(DataSet& data, QTextStream& stream, Parameters params, QString location, int preview_lines)
{
	data.clear(false);

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

	QStringList comments;
	QStringList filters;
	bool is_first_content_line = true;
	int cols = -1;
	int row = 0;
	while (!stream.atEnd() && (preview_lines==-1 || row < preview_lines))
	{
		QString line = stream.readLine();

		//skip empty lines
		if (line.trimmed()=="") continue;

		//skip comment lines
		if (comment_char!=QChar::Null && line[0]==comment_char)
		{
			if (line.startsWith("##TSVVIEW-FILTER##"))
			{
				filters << line;
			}
			else
			{
				comments << line;
			}
			continue;
		}

		//skip first line if requested
		if (first_line_is_comment && is_first_content_line)
		{
			is_first_content_line = false;
			comments << line;
			continue;
		}

		//split
		QStringList parts = line.split(separator);

		//first content line > init columns
		if (cols==-1)
		{
			cols = parts.size();
			for (int i=0; i<cols; ++i)
			{
				numeric_columns << i;
				data.addColumn("", QVector<QString>(), false);
			}
		}

		//check number of elements is correct
		if (parts.count()!=cols)
		{
			THROW(FileParseException, "Number of columns differs in " + location + "!\nExpected " + QString::number(cols) + ", but found " + QString::number(parts.count()) + " in line " + QString::number(row+1) + ":\n" + line);
		}

		//handle quotes
		if (has_quote)
		{
			QString part = parts[0];
			parts[0] = part.mid(1);
			part = parts[parts.count()-1];
			parts[parts.count()-1] = part.mid(0, part.size()-1);
		}

		//add data to columns
		for (int c=0; c<cols; ++c)
		{
			data.column(c).appendString(parts[c]);
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

	//get colmun headers from the first comment line that has the correct count
	foreach(QString comment_line, comments)
	{
		//remove comment character if present
		if (comment_char!=QChar::Null)
		{
			comment_line = comment_line.mid(1);
		}

		//skip empty and double-comment lines
		if (comment_line.trimmed().isEmpty() || comment_line[0]==comment_char)
		{
			continue;
		}

		//split
		QStringList parts = comment_line.split(separator);

		//use if it has the right size
		if (parts.size()==cols)
		{
			//handle quotes
			if (has_quote)
			{
				QString part = parts[0];
				parts[0] = part.mid(1);
				part = parts[parts.count()-1];
				parts[parts.count()-1] = part.mid(0, part.size()-1);
			}

			for (int c=0; c<cols; ++c)
			{
				data.column(c).setHeader(parts[c]);
			}
			break;
		}
	}

	//add comments
	data.setComments(comments);

	//convert numeric columns
	foreach(int c, numeric_columns)
	{
		data.convertStringToNumeric(c);
		data.column(c).autoFormat();
	}

	//apply filters
	QStringList filter_errors;
	foreach (QString line, filters)
	{
		line = line.trimmed();

		int col = -1;
		Filter filter = Filter::fromString(line, col);
		if (filter.type()==Filter::NONE)
		{
			filter_errors << "Unparsable filter line: " + line;
		}
		else if (col<0 || col>=data.columnCount())
		{
			filter_errors << "Filter line with invalid column index: " + line;
		}
		else
		{
			try
			{
				data.column(col).setFilter(filter);
			}
			catch (FilterTypeException& e)
			{
				filter_errors << "Filter line not matching column type: " + line;
			}
		}
	}
	//show filter errors
	if (!filter_errors.isEmpty())
	{
		QMessageBox::warning(QApplication::activeWindow(), "Filter errors", filter_errors.join("\n"));
	}

	data.setModified(false);
}

void TextFile::store(DataSet& data, QString filename, Parameters params)
{
	// open file
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
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

	//write filters
	if (comment=='#')
	{
		for (int col=0; col<cols; ++col)
		{
			if (data.column(col).filter().type()!=Filter::NONE)
			{
				stream << data.column(col).filter().toString(col) << "\n";
			}
		}
	}

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
