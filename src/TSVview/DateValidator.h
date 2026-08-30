#ifndef DATEVALIDATOR_H
#define DATEVALIDATOR_H

#include <QValidator>

class DateValidator : public QValidator
{
public:
	explicit DateValidator(QObject* parent = nullptr);

	State validate(QString& input, int& pos) const;
};

#endif // DATEVALIDATOR_H
