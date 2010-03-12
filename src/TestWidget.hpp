#ifndef QV_BASIC_WIDGETS_H
#define QV_BASIC_WIDGETS_H

#include <QObject>

#include <QWidget>

#ifdef QT_OPENGL_LIB
#include <QGLWidget>
#endif

#include <R.h>
#include <Rinternals.h>

namespace Qanviz {

  enum Operation {
    Glyph,
    Polygon,
    Rectangle,
    Point,
    Line,
    Segment,
    Circle,
    Text
  };
  
  /* Simple widget for testing performance of the drawing layer */
  class TestWidget : public QWidget
  {
  public:
    TestWidget(Operation op, bool antialias)
      : operation(op), antialias(antialias) { }
  protected:
    void paintEvent(QPaintEvent *event);
  private:
    Operation operation;
    bool antialias;
  };

#ifdef QT_OPENGL_LIB
  class TestGLWidget : public QGLWidget // or QGLWidget
  {
  public:
    TestGLWidget(Operation op, bool antialias)
      : QGLWidget(QGLFormat(antialias ? QGL::SampleBuffers : 0)),
        operation(op) { }
  protected:
    void paintEvent(QPaintEvent *event);
  private:
    Operation operation;
  };
#endif
}

#endif
