#ifndef OPENGLPAINTER_H
#define OPENGLPAINTER_H

#ifdef QT_OPENGL_LIB

#include "QtPainter.hpp"
#include <QGLWidget>
#include <QGLFramebufferObject>

namespace Qanviz {
  class OpenGLPainter : public QtPainter {
    
  private:
    
    QGLContext *context;
    
    void setColor(QColor c) {
      float alpha = c.alphaF() * painter->opacity();
      glColor4f(c.redF() * alpha, c.greenF() * alpha, c.blueF() * alpha, alpha);
    }
    GLfloat maxPointSize() {
      GLfloat p[2];
      glGetFloatv(GL_POINT_SIZE_RANGE, p);
      return p[1];
    }
    void drawMidpointCircle(int cx, int cy, uint r, bool fill);
    void drawVertices(GLenum mode, double *x, double *y, int n);
    
    void enableTransform(bool enable = true) {
      if (painter->worldMatrixEnabled() != enable) {
        painter->setWorldMatrixEnabled(enable);
        // HACK: by calling save/restore we force the state update
        painter->save();
        painter->restore();
      }
    }

    void beginCustom();
    void endCustom();
    
  public:

    OpenGLPainter(QGLWidget *widget)
      : QtPainter(widget), context((QGLContext *)widget->context()) { }
      
    OpenGLPainter(QGLFramebufferObject *fbo, QGLContext *context)
      : QtPainter(fbo), context(context) { }

    OpenGLPainter(QPainter *painter, QGLContext *context)
      : QtPainter(painter), context(context) { }

    virtual ~OpenGLPainter() { }

    // Qt 4.5 would benefit from a fast path
    void drawPoints(double *x, double *y, int n);
    
    // if drawing many circles of same size, use drawGlyphs
    void drawCircle(double x, double y, int r);

    // GL_QUADS optimization for filled rectangles
    void drawRectangles(double *x, double *y, double *w, double *h, int n);


    QImage rasterizeGlyph(const QPainterPath &path);
    void drawGlyphs(const QPainterPath &path, double *x, double *y,
                    double *size, QColor *stroke,
                    QColor *fill, int n);
    void drawGlyphs(const QImage &image, double *x, double *y, int n) {
      QtBasePainter::drawGlyphs(image, x, y, n);
    }
    
  protected:
    void prepareDrawGlyphs(void);
    void finishDrawGlyphs(void);
    void drawSomeGlyphs(const QImage &image, double *x, double *y, int n);
  };
}

#endif

#endif
