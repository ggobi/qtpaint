#ifndef QV_BASIC_WIDGETS_H
#define QV_BASIC_WIDGETS_H

#include <QObject>
#include <QGLWidget>

#include <R.h>
#include <Rinternals.h>

namespace Qanviz {
  /* Simple widget for testing performance of the drawing layer */
  class TestWidget : public QGLWidget // or QGLWidget
  {
    Q_OBJECT
    public:
    TestWidget() : QGLWidget(QGLFormat(QGL::SampleBuffers)) { }
    protected:
    void paintEvent(QPaintEvent *event);
  };
  
}

#endif
