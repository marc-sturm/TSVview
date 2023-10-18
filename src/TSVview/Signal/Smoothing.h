#ifndef SMOOTHING_H
#define SMOOTHING_H

#include <QVector>
#include <QVarLengthArray>

#include "Parameters.h"

class Smoothing
{
public:
	enum Type
		{
		MovingAverage,
		MovingMedian,
		SavitzkyGolay
		};

	static void smooth(QVector<double>& data, Type type, Parameters params);

	static Parameters defaultParameters(Type type);

protected:
	static void movingAverage_(QVector<double>& data, Parameters params);
	static void movingMedian_(QVector<double>& data, Parameters params);
	static void savitzkyGolay_(QVector<double>& data, Parameters params);
	static QVarLengthArray<double> getSGCoeffs_(int window_size);

private:
	Smoothing();
};

#endif
