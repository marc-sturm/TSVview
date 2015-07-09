# TSVview
TSV viewer with some editing and statistics functionality

TSVview is a viewer for matrix data, e.g. from tab-separated text files. The program is implemented in C++ using the [Qt](http://qt-project.org/) and [Qwt](http://qwt.sourceforge.net/) libraries.


![Alt text](/doc/TSVview.png)


Features:

 * Free to use for everyone.
 * Portable Windows executable (does not require an installation, just unzip it).
 * Import of character-delimited text files and XML files.
 * Filtering of data according to numeric and string filters.
 * Plotting of data (line plot, histogram, scatter plot, box plot).
 * Sorting of data according to column values.
 * Basic statistics of column values.
 * Own ZXV (Zipped XML Value) data format that allows to store filter along with the data.

### Building

TSVview can be built on Windows only.  
Just clone the repository and open the `src/TSVview.pro` file in QtCreator.

