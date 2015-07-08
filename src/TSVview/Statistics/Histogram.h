#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QVector>

class Histogram
{
public:
	/// Default constructor
	Histogram();
	/// (Re-)initializes the histogram with the given range and bin size
	void init(double min, double max, double bin_size, bool percentage_mode = false);

	/// Increases the bin corresponding to value @p val by one
	void inc(double val, bool ignore_bounds_errors = false);
	/// Increases the bin corresponding to the values in @p data by one
	void inc(QVector<double> data, bool ignore_bounds_errors = false);

	/// Returns the lower bound position (x-axis)
	double min() const;
	/// Returns the upper bound position (x-axis)
	double max() const;
	/// Returns the bin size
	double binSize() const;
	/// Returns the number of bins
	int binCount() const;
	/// Returns the sum of all bins (i.e. the number of data points added)
	int binSum();

	/// Returns the highest value of all bins (y-axis)
	double maxValue() const;
	/// Returns the lowest value of all bins (y-axis)
	double minValue() const;

	/// Returns the value of the bin corresponding to the position @p val
	double binValue(double val) const;
	/// Returns the value of the bin with the index @p index
	double binValue(int index) const;

	/// Returns the center position of the bin with the index @p index
	double centerOfBin(int index) const;

	/// Returns the histogram in string representation
	QString toString() const;

protected:
	/// lower bound position
	double min_;
	/// upper bound position
	double max_;
	/// bin size
	double bin_size_;
	/// percentage mode
	bool percentage_mode_;
	/// sum of all bins (used for percentage mode)
	int bin_sum_;

	/// vector of bins
	QVector<double> bins_;

	/// Returns the bin a given position belongs to
	int valToBin_(double val) const;
};

#endif
