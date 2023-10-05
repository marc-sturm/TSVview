#ifndef TEXTFILE_H
#define TEXTFILE_H

#include "DataSet.h"
#include "Parameters.h"

class TextFile
{
public:
	static void load(DataSet& data, QString filename, Parameters params = Parameters(), int preview_lines = -1);
	static void store(DataSet& data, QString filename, Parameters params = Parameters());

	static Parameters defaultParameters();

	static void fromStream(DataSet& data, QTextStream& stream, QString location, Parameters params = Parameters(), int preview_lines = -1);

private:
	static void determineColumns(DataSet& data, QTextStream& stream, const Parameters& params, int preview_lines);
	static void fromStream(DataSet& data, QTextStream& stream, Parameters params, QString location, int preview_lines);
	TextFile();
};

#endif
