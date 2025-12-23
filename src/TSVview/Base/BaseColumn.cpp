#include "BaseColumn.h"
#include <QRegularExpression>

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
