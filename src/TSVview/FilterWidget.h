#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QDockWidget>

#include "Filter.h"
#include "DataSet.h"
#include "ui_FilterWidget.h"

class FilterWidget
  : public QDockWidget
{
  Q_OBJECT

  public:
    FilterWidget(QWidget *parent = 0);

    void renderFilters(DataSet& dataset);

  signals:
    void filterEnabledChanged(bool);
    void editFilter(int column);
    void removeFilter(int column);
    void reduceToFiltered();
    void removeAllFilters();
    void loadFilter();
    void storeFilter();
    void deleteFilter();

  protected slots:
    void contextMenu(QPoint pos);
    void editFilter(QModelIndex index);

  private:
    Ui::FilterWidget ui_;

};

#endif // FILTERWIDGET_H
