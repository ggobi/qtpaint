#ifndef QV_BASIC_WIDGETS_H
#define QV_BASIC_WIDGETS_H

#include <QObject>
#include <QWidget>
#ifdef QT_OPENGL_LIB
#include <QGLWidget>
#endif

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
    TestWidget(Operation op, bool antialias, bool filled)
      : operation(op), antialias(antialias), filled(filled) { }
  protected:
    void paintEvent(QPaintEvent *event);
  private:
    Operation operation;
    bool antialias;
    bool filled;
  };

#ifdef QT_OPENGL_LIB
  class TestGLWidget : public QGLWidget // or QGLWidget
  {
  public:
    TestGLWidget(Operation op, bool antialias, bool filled)
      : QGLWidget(antialias ? QGLFormat(QGL::SampleBuffers) : QGLFormat()),
        antialias(antialias), operation(op), filled(filled) { }
  protected:
    void paintEvent(QPaintEvent *event);
  private:
    Operation operation;
    bool antialias;
    bool filled;
  };
#endif
}

#endif
