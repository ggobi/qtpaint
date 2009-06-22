
/* Qt-level widgets.  Instructions to render are typically given when
   the widget is constructed, in the form of a SEXP render_info
   (created by R-level wrappers with panel function-like
   interfaces)  */



#ifndef QV_BASIC_WIDGETS_H
#define QV_BASIC_WIDGETS_H

#include <QObject>
#include <QWidget>

#include <QString>
#include <QGridLayout>

#include <R.h>
#include <Rinternals.h>


class QVBasicWidget : public QWidget
{
  Q_OBJECT

  private:

  int r_reference;

  int xexpand;
  int yexpand;

  double x1;
  double x2;
  double y1;
  double y2;

  SEXP render_info;
  QGridLayout *glayout;
  // size policy?

public:

  QVBasicWidget(SEXP rinfo = R_NilValue,
                double x1 = 0., double x2 = 1.,
                double y1 = 0., double y2 = 1.,
                int xexpand = 1, int yexpand = 1,
                int minheight = 0, int minwidth = 0,
                int margin = 0,
                QWidget *parent = 0);
  ~QVBasicWidget();

  double getX1() { return x1; }
  double getX2() { return x2; }
  double getY1() { return y1; }
  double getY2() { return y2; }
  SEXP getRenderInfo() { return render_info; }
  void setX1(double x) { x1 = x; }
  void setX2(double x) { x2 = x; }
  void setY1(double x) { y1 = x; }
  void setY2(double x) { y2 = x; }
  void setRenderInfo(SEXP x) { render_info = x; }
  void setExpansion(int x, int y);
  QGridLayout *getLayout() { return glayout; }
  int getReferenceFlag() { return r_reference; }
  void setReferenceFlag(int i) { r_reference = i; }


  /*  public slots: */

  /*     void print(); */

protected:

  void paintEvent(QPaintEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);

};


#endif

