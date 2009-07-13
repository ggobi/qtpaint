#include <QGLWidget>

#include "PlotView.hpp"
#include <qtbase.h>

using namespace QViz;

extern "C" {

  static QGLWidget *createGLWidget() {
    return new QGLWidget(QGLFormat(QGL::SampleBuffers));
  }
  
  SEXP
  qt_qplotView(SEXP rscene, SEXP rescaleMode, SEXP ropengl)
  {
    SEXP ans;
    QGraphicsScene *scene = unwrapQObject(rscene, QGraphicsScene);
    QGraphicsView *view =
      new QViz::PlotView(scene, 0,
                         (QViz::PlotView::RescaleMode)asInteger(rescaleMode));
    if (asLogical(ropengl))
      view->setViewport(createGLWidget());
    view->setFrameStyle(QFrame::NoFrame);
    ans = wrapQWidget(view);
    addQObjectReference(scene, view); // leaky -- view may change scene
    return ans;
  }
    
  SEXP qt_qviewportRect_QGraphicsView(SEXP rself) {
    QGraphicsView *view = unwrapQObject(rself, QGraphicsView);
    return asRRectF(view->viewport()->rect());
  }

  SEXP qt_qoverlay_PlotView(SEXP rself) {
    PlotView *self = unwrapQObject(rself, PlotView);
    return wrapQObject(self->overlay());
  }

  SEXP qt_qsetOpengl(SEXP rself, SEXP renabled) {
    QGraphicsView *view = unwrapQObject(rself, QGraphicsView);
    if (asLogical(renabled))
      view->setViewport(createGLWidget());
    else view->setViewport(new QWidget);
    return rself;
  }
}


