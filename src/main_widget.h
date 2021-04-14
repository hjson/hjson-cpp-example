#ifndef MAINWIDGET_FJEWIFNNVBAL_H
#define MAINWIDGET_FJEWIFNNVBAL_H

#include "hjson.h"
#include <QWidget>


class QDialog;


class MainWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MainWidget(Hjson::Value&);

public slots:
  void work_finished();

private:
  QDialog *m_dialog;
};


#endif
