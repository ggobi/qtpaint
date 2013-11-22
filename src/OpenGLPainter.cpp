#ifdef QT_OPENGL_LIB

#define GL_GLEXT_PROTOTYPES 1
#include "OpenGLPainter.hpp"
#include <QVarLengthArray>

using namespace Qanviz;

/* Implementation of operations, here vs. QPainter:
   
   1) Line dashing [qpainter]: setDashes would be limited due to
   glLineStipple(). Yes, QPainter would tesselate, but how often does
   one use weird lines??
   2) Line drawing [qpainter]: drawing polyines and segments will not
   be much faster here. OpenGL draws lines in a weird way, and
   QPainter corrects for that.
   3) Points [here]: Qt >=4.5 is slow, use GL_POINTS fast path
   4) Rectangles [both]: GL_QUADS optimization possible in certain cases
   5) Circles [both]: the midpoint algorithm is a hack, but the anti-aliased
   point is a nice trick -- but most of the time we use drawGlyphs. It
   seems QPainter tesselates here, so we should try to optimize.
   6) Polygons [qpainter]: probably same speed, except QPainter handles concave
   polygons, while we are limited to convex cases.
   7) Text [qpainter]: We are already using a painter for this
   8) Images [qpainter]: Probably about the same
   9) Plot glyphs [here]: This is our key optimization.
 */

/* Notes about OpenGL2 engine in Qt 4.6:

   Qt 4.6 adds the OpenGL2 engine, which is designed for OpenGL ES
   2.0. This means there is a heavy reliance on shaders (for embedded
   devices computation is much faster on the GPU). Unfortunately, for
   the types of drawing we do, i.e. lots of points and lines, the
   original paint engine is much faster, as most everything happens
   directly in the hardware. There are some cases (dashed lines,
   concave polygons, text) where we would not expect much difference.

   The obvious downside is that it is unclear whether the OpenGL 1.x
   engine will continue to be maintained and included within Qt.

   Should run performance tests to make sure that OpenGL2 is slower
   when drawing lines and polygons, the cases where we count on Qt
   being fast. Indeed, this is the case, by 20-30X.
 */

void OpenGLPainter::drawPoints(double *x, double *y, int n) {
  beginCustom();
  glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
  if (antialias())
    glEnable(GL_POINT_SMOOTH);
  glPointSize(1);
  setColor(strokeColor());  
  drawVertices(GL_POINTS, x, y, n);
  glPopAttrib();
  endCustom();
}

// if drawing many circles of same size, use drawGlyphs
void OpenGLPainter::drawCircle(double x, double y, int r) {
  
  /* Optimizations:
     1) If radius is within point size range and circle is filled:
     Draw an anti-aliased point.
     This takes about half the time as drawing the circle via
     midpoint, and it looks prettier. There is a lot of overhead that
     could be avoided if many circles were drawn with the same
     radius, but that is the domain of the glyph rendering.

     2) Otherwise, draw circle via midpoint algorithm, which is very
     likely faster than Qt's tesselation (though very large circles
     may start to lose out?).

     Restrictions:
     1) Neither optimization handles dashed lines.
     2) Midpoint only draws aliased lines of width 1.
     3) Neither gracefully handles different fill and stroke colors.
  */

  static GLfloat mps = maxPointSize();

  QPen pen = this->pen();
  QBrush brush = this->brush();
  bool has_pen = pen.style() == Qt::SolidLine;
  bool has_fill = brush.style() != Qt::NoBrush;
  bool same_color = pen.color() == brush.color();
  int lineWidth = pen.width() ? pen.width() : 1;
  int d = 2*r;
  if (has_pen)
    d += lineWidth;

  if (pen.style() == Qt::CustomDashLine || (has_pen && has_fill && !same_color)
      || ((!has_fill || mps < d) && lineWidth > 1))
    QtPainter::drawCircle(x, y, r);
  else if (has_fill && mps >= d) {
    enableTransform();
    beginCustom();
    // FIXME: need to check for POINT_SMOOTH support
    glEnable(GL_POINT_SMOOTH);
    setColor(brush.color());
    glPointSize(d);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
    glDisable(GL_POINT_SMOOTH);
    endCustom();
  } else if (has_pen || has_fill) {
    enableTransform(false);
    beginCustom();
    QPointF p = transform().map(QPointF(x, y));
    if (has_pen)
      setColor(pen.color());
    else setColor(brush.color());
    drawMidpointCircle(p.x(), p.y(), d/2, has_fill);
    endCustom();
  }
}

void OpenGLPainter::drawMidpointCircle(int cx, int cy, uint r, bool fill) {
  /* midpoint circle algorithm, implementation adapted from:
     http://en.wikipedia.org/wiki/Midpoint_circle_algorithm */
  /* WISH: some way to guess final value of 'x' to allocate an array */
  
  int f = 1 - (int)r;
  int ddF_x = 1;
  int ddF_y = -2 * (int)r;
  int x = 0;
  int y = (int)r, fy;
  int tx = 0;
  QVarLengthArray<int, 1024> v;
  int j = 0;
  
  if (fill) { // not clear why this correction is needed
    cy++;
  }
  
  while(x < y) {  /* need to preallocate array */
    if(f >= 0) {
     y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  }
  
  if (!fill) {
    tx = 2*x;
    v.resize(8*(tx+1));
    v[0] = cx; v[1] = cy + (int)r;
    v[2] = cx; v[3] = cy - (int)r;
    v[4] = cx + (int)r; v[5] = cy;
    v[6] = cx - (int)r; v[7] = cy;
    j = 8;
  } else {
    int len = 8*x + 8*(r - y + 1) + 4;
    v.resize(len);
    v[len - 4] = cx - (int)r; v[len - 3] = cy;
    v[len - 2] = cx + (int)r + 1; v[len - 1] = cy;
  }

  y = (int)r;
  f = 1 - (int)r;
  ddF_x = 1;
  ddF_y = -2 * (int)r;
  x = 0;
  
  while(x < y) {
    if(f >= 0) {
      if (fill) {
        v[j++] = cx - x; v[j++] = cy + y;
        v[j++] = cx + x + 1; v[j++] = cy + y;
        v[j++] = cx - x; v[j++] = cy - y;
        v[j++] = cx + x + 1; v[j++] = cy - y;
      }
      y--;
      fy--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++; // not clear why 'fill' correction needed here
    v[j++] = cx + y + (int)fill; v[j++] = cy + x;
    v[j++] = cx - y; v[j++] = cy + x;
    v[j++] = cx + y + (int)fill; v[j++] = cy - x;
    v[j++] = cx - y; v[j++] = cy - x;
    if (!fill) {
      v[j++] = cx - x; v[j++] = cy + y;
      v[j++] = cx + x; v[j++] = cy + y;
      v[j++] = cx - x; v[j++] = cy - y;
      v[j++] = cx + x; v[j++] = cy - y;
    }
    ddF_x += 2;
    f += ddF_x;
  }
  
  if (r > 0 && fill) {
    v[j++] = cx - x; v[j++] = cy + y;
    v[j++] = cx + x; v[j++] = cy + y;
    v[j++] = cx - x; v[j++] = cy - y;
    v[j++] = cx + x; v[j++] = cy - y;
  }

  glVertexPointer(2, GL_INT, 0, v.data());

  glEnableClientState(GL_VERTEX_ARRAY);
  if (fill)
    glDrawArrays(GL_LINES, 0, v.size()/2);
  else glDrawArrays(GL_POINTS, 0, v.size()/2);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void OpenGLPainter::drawRectangles(double *x, double *y, double *w, double *h,
                                   int n)
{
  if (hasFill() &&
      (!hasStroke() || (strokeColor() == fillColor() &&
                        pen().style() == Qt::SolidLine &&
                        lineWidth() == 0)))
    {
      QVarLengthArray<double, 4096> vertices(n*8);
      double *v = vertices.data();
      for (int i = 0; i < n; i++) {
        v[0] = x[i]; v[1] = y[i];
        v[2] = x[i] + w[i]; v[3] = y[i];
        v[4] = x[i] + w[i]; v[5] = y[i] + h[i];
        v[6] = x[i]; v[7] = y[i] + h[i];
        v += 8;
      }
      beginCustom();
      setColor(fillColor());
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
      glDrawArrays(GL_QUADS, 0, 4*n);
      glDisableClientState(GL_VERTEX_ARRAY);
      endCustom();
    } else QtPainter::drawRectangles(x, y, w, h, n);
}

// We force the glyph into a square by copying to square image with offset
QImage OpenGLPainter::rasterizeGlyph(const QPainterPath &path) {
  QImage glyph = QtBasePainter::rasterizeGlyph(path);
  int width = glyph.width(), height = glyph.height();
  if (width != height) {
    int maxDim = qMax(width, height);
    QImage square(maxDim, maxDim, glyph.format());
    square.fill(0);
    QPainter painter(&square);
    painter.drawImage((maxDim - width)/2, (maxDim - height)/2, glyph);
    glyph = square;
  }
  return glyph;
}

/* FIXME: what about using z-level and GL_DEPTH_TEST? */

void OpenGLPainter::drawGlyphs(const QPainterPath &path, double *x, double *y,
                               double *size, QColor *stroke,
                               QColor *fill, int n)
{
  // FIXME: could we make this work even if size != NULL?
  if (!size && (stroke || fill)) {
    bool do_stroke = stroke || hasStroke(), do_fill = fill || hasFill();
    bool fast = !do_stroke || !do_fill;
    if (!fast) {
      int i = 0;
      bool equal = true;
      if (stroke && fill) {
        for (i = 0; i < n && equal; i++)
          equal = stroke[i] == fill[i];
      } else if (stroke && do_fill) {
        QColor fill = fillColor();
        for (i = 0; i < n && equal; i++)
          equal = stroke[i] == fill;
      } else if (fill && do_stroke) {
        QColor stroke = strokeColor();
        for (i = 0; i < n && equal; i++)
          equal = stroke == fill[i];
      }
      fast = i == n;
    }
    if (fast) {
      QColor prevStroke = strokeColor(), prevFill = fillColor();
      if (do_stroke) setStrokeColor(Qt::white);
      if (do_fill) setFillColor(Qt::white);
      qreal prevOpacity = opacity();
      setOpacity(1.0);
      QImage glyph = rasterizeGlyph(path);
      /*      
      for (int i = 0; i < glyph.height(); i++)
        for (int j = 0; j < glyph.bytesPerLine()/4; j++)
          printf("%x,", glyph.pixel(j, i));
      */
      if (do_stroke) setStrokeColor(prevStroke);
      if (do_fill) setFillColor(prevFill);
      setOpacity(prevOpacity);
      prepareDrawGlyphs();
      // override the env mode for modulation
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      QVector<float> colors(n*4);
      float *tmp = colors.data();
      if (!stroke)
        stroke = fill;
      for (int i = 0; i < n; i++, tmp += 4) {
        //printf("%x\n", stroke[i].rgba());
        float alpha = stroke[i].alphaF() * opacity();
        tmp[3] = alpha;
        tmp[2] = stroke[i].blueF() * alpha;
        tmp[1] = stroke[i].greenF() * alpha;
        tmp[0] = stroke[i].redF() * alpha;
      }
      glColorPointer(4, GL_FLOAT, 0, colors.data());
      glEnableClientState(GL_COLOR_ARRAY);
      drawSomeGlyphs(glyph, x, y, n);
      glDisableClientState(GL_COLOR_ARRAY);
      finishDrawGlyphs();
      return;
    }
  }
  QtBasePainter::drawGlyphs(path, x, y, size, stroke, fill, n);
}

void OpenGLPainter::prepareDrawGlyphs(void) {
  beginCustom();
  glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_POINT_BIT);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  /*
    FIXME: point sprites, after a long period of deprecation, have
    been removed with OpenGL 3.2. The alternative is to use a shader
    program, like:
    void main()
    {
    gl_FragColor = texture2D(Texture0, gl_PointCoord);
    }
   */
  /*
    Point sprites do not exist in OpenGL 1.1 (Windows). Need to
    dynamically obtain the function pointers via the extensions API.
   */
  #ifdef Q_OS_WIN
  PFNGLPOINTPARAMETERIPROC glPointParameteri;
  glPointParameteri = (PFNGLPOINTPARAMETERIPROC)
    QGLContext::currentContext()->getProcAddress("glPointParameteri");
  #endif
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
  enableTransform();
}
void OpenGLPainter::finishDrawGlyphs(void) {
  glPopAttrib();
  endCustom();
}

void OpenGLPainter::drawSomeGlyphs(const QImage &image, double *x, double *y,
                                   int n)
{
  GLuint tex = context->bindTexture(image
#if QT_VERSION >= 0x40600
                                    , GL_TEXTURE_2D, GL_RGBA,
                                    QGLContext::PremultipliedAlphaBindOption
#endif
                                    );
  glPointSize(image.width());
  drawVertices(GL_POINTS, x, y, n);
  context->deleteTexture(tex);
}

void OpenGLPainter::drawVertices(GLenum mode, double *x, double *y, int n) {
  QVarLengthArray<double, 4096> vertices(n*2);
  for (int i = 0; i < n; i++) {
    vertices[i*2] = x[i];
    vertices[i*2+1] = y[i];
  }
  glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawArrays(mode, 0, n);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void OpenGLPainter::beginCustom() {
#if QT_VERSION >= 0x40600
  painter->beginNativePainting();
#endif
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLPainter::endCustom() {
  glPopAttrib();
#if QT_VERSION >= 0x40600
  painter->endNativePainting();
#endif
}

#endif
