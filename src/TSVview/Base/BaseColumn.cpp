#include "BaseColumn.h"
#include <QRegularExpression>
#include "Exceptions.h"

BaseColumn::BaseColumn(Type type)
  : QObject(0)
  , header_()
	, type_(type)
  , filter_()
{
}

BaseColumn::BaseColumn(const BaseColumn& rhs)
  : QObject(0)
  , header_(rhs.header_)
  , type_(rhs.type_)
  , filter_(rhs.filter_)
{
}

QString BaseColumn::headerOrIndex(int index, bool force_index) const
{
  if (header_=="")
  {
    return "[" + QString::number(index) + "]";
  }

  if (force_index)
  {
    return "[" + QString::number(index) + "] " + header_;
  }

  return header_;
}

bool BaseColumn::setHeader(const QString& header)
{
  if (header==header_)
  {
    return true;
  }

  //check if name contains only valid characters
  if (!QRegularExpression("[A-Za-z0-9_ +/.:äöüÄÖÜß)(#-]*").match(header).hasMatch())
  {
    return false;
  }

	header_ = header;

	emit headerChanged();

  return true;
}

QString BaseColumn::typeToString(Type type)
{
    if (type==BaseColumn::NUMERIC) return "numeric";
    else if (type==BaseColumn::STRING) return "string";
    else THROW(ProgrammingException, "Unhandled column type "+QString::number(type));
}

BaseColumn::Type BaseColumn::stringToType(QString str)
{
    if (str=="numeric") return BaseColumn::NUMERIC;
    else if (str=="string") return BaseColumn::STRING;
    else THROW(ProgrammingException, "Unhandled column name "+str);
}
