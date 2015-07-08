#ifndef CUSTOM_EXCEPTIONS_H
#define CUSTOM_EXCEPTIONS_H

#include "Exceptions.h"

/// Exception for import error
class FileIOException
		: public Exception
{
public:
	FileIOException(QString message, QString file, int line);
};

/// Exception for parameter handling errors
class ParameterException
		: public Exception
{
public:
	ParameterException(QString message, QString file, int line);

};

/// Exception for signal processing calculations
class SignalProcessingException
		: public Exception
{
public:
	SignalProcessingException(QString message, QString file, int line);
};

/// Exception in case of wrong filter type
class FilterTypeException
		: public Exception
{
public:
	FilterTypeException(QString message, QString file, int line);
};

/// Exception in case of unmatching find type
class FindTypeException
		: public Exception
{
public:
	FindTypeException(QString message, QString file, int line);
};

/// Exception used to abort actions, but clean up
class CleanUpException
		: public Exception
{
public:
	CleanUpException();
};

#endif
