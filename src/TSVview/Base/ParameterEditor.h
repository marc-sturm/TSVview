#ifndef PARAMETEREDITOR_H
#define PARAMETEREDITOR_H

#include <QWidget>
#include <QStackedWidget>
#include <QComboBox>

#include "Parameters.h"

class ParameterEditor
		: public QWidget
{
	Q_OBJECT

public:
	ParameterEditor(QWidget* parent = 0);

	void setParameters(Parameters& parameters);

	static bool asDialog(QIcon icon, QString window_title, Parameters& parameters);

protected slots:
	void change_(double value);
	void change_(int value);
	void change_(const QString& value);
	void change_(bool value);
	void change_(QColor value);
	void changeString_(int value);
	void changeChar_(int value);
	void changeChar_(const QString& value);

private:
	QComboBox* combo_;
	QStackedWidget* stack_;
	Parameters* params_;

	void setupUi();
	QWidget* createWidget_(QString key, QWidget* parent);
};

#endif
