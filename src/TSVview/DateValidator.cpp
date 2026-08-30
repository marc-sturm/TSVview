#include "DateValidator.h"
#include <QDate>

DateValidator::DateValidator(QObject* parent)
	: QValidator{parent}
{
}

QValidator::State DateValidator::validate(QString& input, int& pos) const
{
	Q_UNUSED(pos);

	//empty
	if (input.isEmpty()) return Intermediate;

	//while typing
	static QRegularExpression re(R"(^\d{0,4}(-\d{0,2}(-\d{0,2})?)?$)");
	if (!re.match(input).hasMatch()) return Invalid;
	if (input.length()<10) return Intermediate;

	//complete > validate
	QDate date = QDate::fromString(input, Qt::ISODate);
	if (date.isValid()) return Acceptable;

	return Invalid;
}

