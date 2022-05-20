#include "CustomExceptions.h"

FileIOException::FileIOException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

ParameterException::ParameterException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

SignalProcessingException::SignalProcessingException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

FilterTypeException::FilterTypeException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

FindTypeException::FindTypeException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

CleanUpException::CleanUpException()
	: Exception("", "none", -1, ExceptionType::CRITIAL)
{
}
