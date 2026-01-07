#include "CustomExceptions.h"

ParameterException::ParameterException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

FilterTypeException::FilterTypeException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}
