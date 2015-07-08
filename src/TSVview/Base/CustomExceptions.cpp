#include "CustomExceptions.h"

FileIOException::FileIOException(QString message, QString file, int line)
	: Exception(message, file, line)
{
}

ParameterException::ParameterException(QString message, QString file, int line)
	: Exception(message, file, line)
{
}

SignalProcessingException::SignalProcessingException(QString message, QString file, int line)
	: Exception(message, file, line)
{
}

FilterTypeException::FilterTypeException(QString message, QString file, int line)
	: Exception(message, file, line)
{
}

FindTypeException::FindTypeException(QString message, QString file, int line)
	: Exception(message, file, line)
{
}

CleanUpException::CleanUpException()
	: Exception("", "none", -1)
{
}
