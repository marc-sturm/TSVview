#include "DataSet.h"
#include <math.h>
#include <algorithm>
#include <QDebug>
#include <BasicStatistics.h>
#include <QMessageBox>
#include "CustomExceptions.h"
#include "VersatileTextStream.h"
#include "Helper.h"
#include <QApplication>

DataSet::DataSet()
	: QObject(0)
	, columns_()
    , modified_(false)
	, filters_enabled_(true)
	, filtered_rows_()
{
	columns_.reserve(100);
}


DataSet::~DataSet()
{
	clear(false);
}

void DataSet::clear(bool emit_signals)
{
    //delete data
	for (int i=0; i<columns_.count(); ++i)
	{
		delete(columns_[i]);
	}
	columns_.clear();
    modified_ = false;
    filters_enabled_ = true;
    filtered_rows_.clear();

	if (emit_signals)
	{
        emit modificationStatusChanged(false);
		emit filtersChanged();
		emit headersChanged();
		emit dataChanged();
	}
}

QStringList DataSet::headers()
{
	QStringList output;
	foreach(BaseColumn* col, columns_)
	{
		output << col->header();
	}
	return output;
}

int DataSet::indexOf(const QString& name)
{
	for (int i=0; i<columns_.count(); ++i)
	{
		if (columns_[i]->header()==name)
		{
			return i;
		}
	}

	return -1;
}

void DataSet::removeColumns(QSet<int> columns)
{
	//special case: removing all columns
	if (columns.count()==columnCount())
	{
		clear(true);
		return;
	}

	//sort coumns in reverse order
	QList<int> column_list = SET_TO_LIST(columns);
    std::sort(column_list.begin(), column_list.end(), std::greater<int>());

	//remove columns
	for (int i=0; i<column_list.count(); ++i)
	{
        int col = column_list[i];
        Q_ASSERT(col<columns_.size());

        delete(columns_[col]);
        columns_.remove(col);
    }

	emit dataChanged();
	emit filtersChanged();
    setModified(true);
}

void DataSet::addColumn(QString header, const QVector<double>& data, const QVector<char>& decimals, int index)
{
    Q_ASSERT(data.size()==decimals.size());
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());

	NumericColumn* new_col = new NumericColumn();
    new_col->setValues(data, decimals);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	if (index<0 || index>=data.size())
	{
		columns_.append(new_col);
	}
	else
	{
		columns_.insert(index, new_col);
	}

    emit dataChanged();
    setModified(true);
}

void DataSet::addColumn(QString header, const QVector<QString>& data, int index)
{
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());

	StringColumn* new_col = new StringColumn();
	new_col->setValues(data);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	if (index<0 || index>=data.size())
	{
		columns_.append(new_col);
	}
	else
	{
		columns_.insert(index, new_col);
	}

    emit dataChanged();
    setModified(true);
}

void DataSet::replaceColumn(int index, QString header, const QVector<double>& data, const QVector<char>& decimals)
{
    Q_ASSERT(data.size()==decimals.size());
	Q_ASSERT(rowCount()==0 || data.size()==rowCount());
	Q_ASSERT(index < data.size());

	NumericColumn* new_col = new NumericColumn();
    new_col->setValues(data, decimals);
	new_col->setHeader(header);

	connect(new_col, SIGNAL(dataChanged()), this, SLOT(columnDataChanged()));
	connect(new_col, SIGNAL(filterChanged()), this, SLOT(filterDataChanged()));
	connect(new_col, SIGNAL(headerChanged()), this, SLOT(headerDataChanged()));

	//replace old column
	BaseColumn* old_col = columns_[index];
	columns_.replace(index, new_col);
	delete old_col;

	emit dataChanged();
	setModified(true);
}

void DataSet::setModified(bool modified, bool force_emit)
{
    bool changed = modified_!=modified;

    modified_ = modified;

    if (changed || force_emit)
    {
		emit modificationStatusChanged(modified_);
	}
}

void DataSet::sortByColumn(int column, bool reverse)
{
	Q_ASSERT(column<columns_.size());

	int size = rowCount();
	QVector<int> indices(size);

    //create temporary vector with column data and index //TODO move to column: QList<int> getSortedOrder() const
	if (columns_[column]->type()==BaseColumn::NUMERIC)
	{
		std::vector<std::pair<double, int> > tmp;
		tmp.reserve(size);
		QVector<double> col = dynamic_cast<NumericColumn*>(columns_[column])->values();
		for (int i=0; i<size; ++i)
		{
			double value = col[i];
			if (!BasicStatistics::isValidFloat(value)) value = std::numeric_limits<double>::max();
			tmp.push_back(std::make_pair(value, i));
		}

		//sort the vector according to the value
		if (!reverse)
		{
			std::sort(tmp.begin(), tmp.end());
		}
		else
		{
			std::sort(tmp.begin(), tmp.end(), std::greater<std::pair<double, int> >());
		}

		for (int i=0; i<size; ++i)
		{
			indices[i] = tmp[i].second;
		}
	}
	else
	{
		std::vector<std::pair<QString, int> > tmp;
		tmp.reserve(size);
		QVector<QString> col = dynamic_cast<StringColumn*>(columns_[column])->values();
		for (int i=0; i<size; ++i)
		{
			tmp.push_back(std::make_pair(col[i], i));
		}

		//sort the vector according to the value
		if (!reverse)
		{
			std::sort(tmp.begin(), tmp.end());
		}
		else
		{
			std::sort(tmp.begin(), tmp.end(), std::greater<std::pair<QString, int> >());
		}

		for (int i=0; i<size; ++i)
		{
			indices[i] = tmp[i].second;
		}
	}

    //use the indices to change the order of all columns //TODO move to column: void reorder(QList<int> order)
	for (int c=0; c<columns_.size(); ++c)
	{
		if (columns_[c]->type()==BaseColumn::NUMERIC)
		{
            NumericColumn* column = dynamic_cast<NumericColumn*>(columns_[c]);
            const QVector<double>& col_values = column->values();
            const QVector<char>& col_decimals = column->decimals();
			QVector<double> new_col(size);
            QVector<char> new_col_dec(size);
			for (int i=0; i<size; ++i)
			{
                new_col[i] = col_values[indices[i]];
                new_col_dec[i] = col_decimals[indices[i]];
			}
            column->setValues(new_col, new_col_dec);
		}
		else
		{
			QVector<QString> col = dynamic_cast<StringColumn*>(columns_[c])->values();
			QVector<QString> new_col(size);
			for (int i=0; i<size; ++i)
			{
				new_col[i] = col[indices[i]];
			}
			dynamic_cast<StringColumn*>(columns_[c])->setValues(new_col);
		}
	}

    emit dataChanged();
    setModified(true);
}

void DataSet::mergeColumns(QList<int> cols, QString header, QString sep)
{
	int first_col = cols.at(0);

	//create new column
	QVector<QString> new_col;
	new_col.reserve(rowCount());
	for (int r=0; r<rowCount(); ++r)
	{
		QString value;
		foreach(int c, cols)
		{
			if (c!=first_col) value.append(sep);
			value.append(column(c).string(r));
		}
		new_col.append(value);
	}

	//remove old columns and add new one
	removeColumns(LIST_TO_SET(cols));
    addColumn(header, new_col, first_col);
}

void DataSet::reduceToRows(QSet<int> rows)
{
	//convert rows set to ordered list
	QList<int> keep_rows = SET_TO_LIST(rows);
	std::sort(keep_rows.begin(), keep_rows.end());

	//update all columns
	blockSignals(true);
	for (int c=0; c<columnCount(); ++c)
	{
		//numeric column
		if (column(c).type()==BaseColumn::NUMERIC)
		{
			NumericColumn& column = numericColumn(c);
            const QList<double>& old_values = column.values();
            const QList<char>& old_decimals = column.decimals();

            QVector<double> values;
			values.reserve(keep_rows.count());
            QVector<char> decimals;
            decimals.reserve(keep_rows.count());
			for (int r=0; r<keep_rows.count(); ++r)
			{
                values << old_values[keep_rows[r]];
                decimals << old_decimals[keep_rows[r]];
            }
            column.setValues(values, decimals);
		}
		//string column
		else
		{
			StringColumn& column = stringColumn(c);
            const QList<QString>& old_values = column.values();

			QVector<QString> values;
			values.reserve(keep_rows.count());
			for (int r=0; r<keep_rows.count(); ++r)
			{
                values << old_values[keep_rows[r]];
			}
			column.setValues(values);
		}
	}
	blockSignals(false);

    emit dataChanged();
    setModified(true, true);
}

void DataSet::convertStringToNumeric(int c)
{
	Q_ASSERT(c>=0);
	Q_ASSERT(c<columns_.size());

	//create numeric data
    const QVector<QString>& values = stringColumn(c).values();
	QVector<double> numbers;
    numbers.reserve(values.count());
    QVector<char> decimals;
    decimals.reserve(values.count());
    foreach(const QString& value, values)
	{
        auto tmp = NumericColumn::toDouble(value);
        numbers << tmp.first;
        decimals << tmp.second;
	}

	//replace string by numeric column
    replaceColumn(c, column(c).header(), numbers, decimals);
}

void DataSet::setFiltersEnabled(bool enabled)
{
	filters_enabled_ = enabled;

	emit filtersChanged();
}

bool DataSet::filtersPresent() const
{
	for (int i=0; i<columnCount(); ++i)
	{
		if (column(i).filter().type() != Filter::NONE)
		{
			return true;
		}
	}

	return false;
}

void DataSet::columnDataChanged()
{
	setModified(true);

	BaseColumn* column = qobject_cast<BaseColumn*>(sender());
	emit columnChanged(columns_.indexOf(column), false);
}

void DataSet::headerDataChanged()
{
	setModified(true);

	emit headersChanged();
}

void DataSet::filterDataChanged()
{
	setModified(true);

	emit filtersChanged();
}

QBitArray DataSet::getRowFilter(bool update) const
{
	if (!filters_enabled_)
	{
		filtered_rows_.fill(true, rowCount());
	}

	if (filters_enabled_ && update)
	{
		filtered_rows_.fill(true, rowCount());

		//determine rows to render
		for (int c=0; c<columnCount(); ++c)
		{
			column(c).matchFilter(filtered_rows_);
		}
	}

	return filtered_rows_;
}

void DataSet::load(QString filename, QString display_name)
{
    if (display_name.isEmpty()) display_name = filename;

    //clear
    clear(true);

    QElapsedTimer timer;
    timer.start();

    QSet<int> numeric_columns;
    QStringList comments;
    QStringList filters;
    int line_nr = -1;
    int cols = -1;
    int rows = -1;
    VersatileTextStream file(filename);
    while (!file.atEnd())
    {
        QString line = file.readLine(true);
        ++line_nr;

        //skip empty lines
        if (line.isEmpty()) continue;

        //header/comment lines
        if (line[0]=='#')
        {
            if (line.startsWith("##")) //comment
            {
                if (line.startsWith("##TSVVIEW-"))
                {
                    if (line.startsWith("##TSVVIEW-FILTER##"))
                    {
                        filters << line;
                    }
                    else if (line.startsWith("##TSVVIEW-ROWS##"))
                    {
                        rows = line.split("##")[2].toInt();
                    }
                }
                else
                {
                    comments << line;
                }
            }
            else //header
            {
                if (cols!=-1) THROW(FileParseException, "Found second header line in " + display_name + ":\n"+line);

                QStringList parts = line.mid(1).split('\t');
                cols = parts.size();
                for (int c=0; c<cols; ++c)
                {
                    numeric_columns << c;

                    QVector<QString> values;
                    if (rows!=-1) values.reserve(rows);
                    addColumn(parts[c], values, false);
                }
            }
            continue;
        }

        //content line
        QStringList parts = line.split('\t');

        //check number of elements is correct
        if (parts.count()!=cols) THROW(FileParseException, "Mixed number of columns in " + display_name + "!\nExpected " + QString::number(cols) + " based on header line, but found " + QString::number(parts.count()) + " in line " + QString::number(line_nr) + ":\n" + line);

        //add data to columns
        for (int c=0; c<cols; ++c)
        {
            column(c).appendString(parts[c]);
        }

        //try to convert numbers
        foreach(int c, numeric_columns)
        {
            if (!Helper::isNumeric(parts[c])) numeric_columns.remove(c);
        }
    }

    //add comments
    setComments(comments);

    qDebug() << "loading data from file: c=" << columnCount() << "r=" << rowCount() << "ms=" << timer.restart();

    //convert numeric columns
    foreach(int c, numeric_columns)
    {
        convertStringToNumeric(c);
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
        else if (col<0 || col>=columnCount())
        {
            filter_errors << "Filter line with invalid column index: " + line;
        }
        else
        {
            try
            {
                column(col).setFilter(filter);
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

    setModified(false);

    qDebug() << "converting numeric columns: ms=" << timer.elapsed();
}

void DataSet::import(QString filename, QString display_name, Parameters params, int preview_lines)
{
    //clear
    clear(true);

    QElapsedTimer timer;
    timer.start();

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
    bool is_first_content_line = true;
    int cols = -1;
    int row = 0;
    VersatileTextStream file(filename);
    while (!file.atEnd() && (preview_lines==-1 || row < preview_lines))
    {
        QString line = file.readLine(true);

        //skip empty lines
        if (line.isEmpty()) continue;

        //skip comment lines
        if (comment_char!=QChar::Null && line[0]==comment_char)
        {
            comments << line;
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
                addColumn("", QVector<QString>(), false);
            }
        }

        //check number of elements is correct
        if (parts.count()!=cols)
        {
            THROW(FileParseException, "Number of columns differs in " + display_name + "!\nExpected " + QString::number(cols) + ", but found " + QString::number(parts.count()) + " in line " + QString::number(row+1) + ":\n" + line);
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
            column(c).appendString(parts[c]);
        }

        //try to convert numbers
        foreach(int c, numeric_columns)
        {
            if (!Helper::isNumeric(parts[c])) numeric_columns.remove(c);
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
                column(c).setHeader(parts[c]);
            }
            break;
        }
    }

    //convert numeric columns
    foreach(int c, numeric_columns)
    {
        convertStringToNumeric(c);
    }

    setModified(true, true);

    qDebug() << "import data: c=" << columnCount() << "r=" << rowCount() << "ms=" << timer.restart();
}

void DataSet::store(QString filename)
{
    QElapsedTimer timer;
    timer.start();

    bool is_gz = filename.endsWith(".gz", Qt::CaseInsensitive);
    if (is_gz)
    {
        storeGzipped(filename);
    }
    else
    {
        storePlain(filename);
    }

    qDebug() << QString("storing")+(is_gz ? " (GZ)" : "")+" ms=" << timer.elapsed();
}

void DataSet::storePlain(QString filename)
{
    // open file
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        THROW(FileAccessException, "Could not open file '" + filename + "' for writing.");
    }

    //write comments
    QTextStream stream(&file);
    foreach(const QString& comment, comments())
    {
        if (comment.startsWith("##TSVVIEW-ROWS##")) continue;
        stream << comment << '\n';
    }
    stream << "##TSVVIEW-ROWS##" << rowCount() << '\n';

    //write filters
    for (int col=0; col<columnCount(); ++col)
    {
        if (column(col).filter().type()!=Filter::NONE)
        {
            stream << column(col).filter().toString(col) << '\n';
        }
    }

    // write header
    stream << '#' << headers().join('\t') << '\n';

    // write data
    for (int row=0; row<rowCount(); ++row)
    {
        if (row!=0)
        {
            stream << '\n';
        }
        for (int col=0; col<columnCount(); ++col)
        {
            if (col!=0)
            {
                stream << '\t';
            }
            stream << column(col).string(row);
        }
    }
}

void DataSet::storeGzipped(QString filename)
{
    // open file
    gzFile file = gzopen(filename.toUtf8().constData(), "wb");
    if (!file) THROW(FileAccessException, "Could not open file '" + filename + "' for writing.");

    //write comments
    foreach(const QString& comment, comments())
    {
        if (comment.startsWith("##TSVVIEW-ROWS##")) continue;
        QByteArray tmp = (comment +'\n').toUtf8();
        gzwrite(file, tmp.constData(), tmp.size());
    }
    QByteArray tmp = ("##TSVVIEW-ROWS##" + QByteArray::number(rowCount()) + '\n');
    gzwrite(file, tmp.constData(), tmp.size());

    //write filters
    for (int col=0; col<columnCount(); ++col)
    {
        if (column(col).filter().type()!=Filter::NONE)
        {
            QByteArray tmp = (column(col).filter().toString(col) + '\n').toUtf8();
            gzwrite(file, tmp.constData(), tmp.size());
        }
    }

    // write header
    tmp = ('#' + headers().join('\t') +'\n').toUtf8();
    gzwrite(file, tmp.constData(), tmp.size());

    // write data
    for (int row=0; row<rowCount(); ++row)
    {
        QByteArray tmp;
        if (row!=0)
        {
            tmp.append('\n');
        }
        for (int col=0; col<columnCount(); ++col)
        {
            if (col!=0)
            {
                tmp.append('\t');
            }
            tmp.append(column(col).string(row).toUtf8());
        }
        gzwrite(file, tmp.constData(), tmp.size());
    }

    gzclose(file);
}

void DataSet::storeAs(QString filename, ExportFormat format)
{
    if (format==ExportFormat::HTML)
    {
        storeAsHtml(filename);
    }
    else if (format==ExportFormat::CSV)
    {
        storeAsCsv(filename);
    }
    else THROW(NotImplementedException, "Invalid format!");
}

void DataSet::storeAsHtml(QString filename)
{
    // open file
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        THROW(FileAccessException, "Could not open file '" + filename + "' for writing.");
    }

    // write header
    QTextStream stream(&file);
    int indent = 0;

    //before table
    writeHtml(stream, indent, "<html>", true);
    indent += 2;
    writeHtml(stream, indent, "<head>", true);
    indent += 2;
    writeHtml(stream, indent, "<style>", true);
    indent += 2;
    writeHtml(stream, indent, "table { border-collapse: collapse; width: auto; border: 1px solid #444; }", true);
    writeHtml(stream, indent, "table td { border: 1px solid #444; padding: 2px; }", true);
    writeHtml(stream, indent, "table th { border: 1px solid #444; text-align: left; padding: 2px; background: #ccc; font-weight: 600; }", true);
    writeHtml(stream, indent, "table tr:nth-child(even) td { background: #f3f3f3; }", true);
    writeHtml(stream, indent, "table tr:hover td { background: #d0d7df; }", true);
    indent -= 2;
    writeHtml(stream, indent, "</style>", true);
    indent -= 2;
    writeHtml(stream, indent, "</head>", true);
    writeHtml(stream, indent, "<body>", true);
    indent += 2;
    writeHtml(stream, indent, "<table>", true);
    indent += 2;

    //header line
    QStringList header_names = headers();
    writeHtml(stream, indent, "<tr>", true);
    indent += 2;
    for(int i=0; i<header_names.count(); ++i)
    {
        writeHtml(stream, indent, "<th>"+header_names[i].toUtf8()+"</th>", true);
    }
    indent -= 2;
    writeHtml(stream, indent, "</tr>", true);

    //content lines
    for (int row=0; row<rowCount(); ++row)
    {
        writeHtml(stream, indent, "<tr>", true);
        indent += 2;
        for(int col=0; col<columnCount(); ++col)
        {
            writeHtml(stream, indent, "<td>"+column(col).string(row).toUtf8()+"</td>", true);
        }
        indent -= 2;
        writeHtml(stream, indent, "</tr>", true);
    }

    //after
    indent -= 2;
    writeHtml(stream, indent, "</table>", true);
    indent -= 2;
    writeHtml(stream, indent, "</body>", true);
    indent -= 2;
    writeHtml(stream, indent, "</html>", true);
}

void DataSet::writeHtml(QTextStream& stream, int indent, QByteArray text, bool newline)
{
    if (indent>0) stream << QByteArray().fill(' ', indent);
    stream << text;
    if (newline) stream << "\n";
}

void DataSet::storeAsCsv(QString filename)
{
    // open file
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        THROW(FileAccessException, "Could not open file '" + filename + "' for writing.");
    }

    // write header
    QTextStream stream(&file);

    QStringList header_names = headers();
    for (int col=0; col<header_names.count(); ++col)
    {
        if (col!=0) stream << ',';
        stream << escapeForCsv(header_names[col]);
    }
    stream << '\n';

    // write data
    for (int row=0; row<rowCount(); ++row)
    {
        if (row!=0) stream << '\n';
        for (int col=0; col<columnCount(); ++col)
        {
            if (col!=0) stream << ',';
            stream << escapeForCsv(column(col).string(row));
        }
    }
}

QString DataSet::escapeForCsv(QString s)
{
    s.replace("\"", "\"\"");
    if (s.contains(',') || s.contains('"') || s.contains('\n'))
    {
        s = "\"" + s + "\"";
    }
    return s;
}
