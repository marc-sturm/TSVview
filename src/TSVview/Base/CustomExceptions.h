#ifndef CUSTOM_EXCEPTIONS_H
#define CUSTOM_EXCEPTIONS_H

#include "Exceptions.h"

/// Exception for parameter handling errors
class ParameterException
		: public Exception
{
public:
	ParameterException(QString message, QString file, int line, ExceptionType type);

};

/// Exception in case of wrong filter type
class FilterTypeException
		: public Exception
{
public:
	FilterTypeException(QString message, QString file, int line, ExceptionType type);
};

#endif
