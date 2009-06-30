#include <QGraphicsScene>
#include <QGraphicsWidget>

#include <qtbase.h>

extern "C" {
  // REMOVE when qtgui is able to represent and manipulate QBrush
  // objects, so that we just set the background brush directly
  SEXP qt_qsetBackgroundColor_QGraphicsScene(SEXP rself, SEXP rcolor) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    scene->setBackgroundBrush(QBrush(asQColor(rcolor)));
    return rself;
  }
}
