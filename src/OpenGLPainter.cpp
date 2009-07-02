#define GL_GLEXT_PROTOTYPES 1
#include "OpenGLPainter.hpp"
#include <QVarLengthArray>

using namespace QViz;

/* Case for just using QPainter for most of this:
   1) Line dashing: setDashes would be limited due to glLineStipple(). Yes,
   QPainter would tesselate, but how often does one use weird lines??
   2) Line drawing: drawing polyines and segments will not be much
   faster here. OpenGL draws lines in a weird way, and QtPainter
   corrects for that.
   3) Points: probably no difference (but in Qt 4.5, there is)
   4) Rectangles: QtPainter is a bit faster
   5) Circles: the midpoint algorithm is a hack, but the anti-aliased
   point is a nice trick -- but most of the time we use drawGlyphs. It
   seems QtPainter tesselates here, so we should try to optimize.
   6) Polygons: probably same speed, except QtPainter handles concave
   polygons, while we are limited to convex cases.
   7) Text: We are already using a painter for this
   8) Images: Probably about the same
   9) Plot glyphs: This is our key optimization. Just override this.

   It seems we could move to QtPainter, without much loss in speed,
   except in cases like: line width > 1 and dashes, where we would be
   faster, but uglier. Users should probably use simple lines for
   exploratory, interactive graphics, then tweak the aesthetics for
   presentation. QPainter would certainly help with concave polygons and
   text. Just need to add glyph blitting as a fast path, and optimize
   circles.
 */

void OpenGLPainter::drawPoints(double *x, double *y, int n) {
  glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
  if (antialias())
    glEnable(GL_POINT_SMOOTH);
  glPointSize(1);
  setColor(strokeColor());  
  drawVertices(GL_POINTS, x, y, n);
  glPopAttrib();
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
      || ((!has_fill || mps < d) && (lineWidth > 1 || antialias())))
    QtPainter::drawCircle(x, y, r);
  else if (has_fill && mps >= d) {
    enableTransform();
    glEnable(GL_POINT_SMOOTH);
    setColor(brush.color());
    glPointSize(d);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
    glDisable(GL_POINT_SMOOTH);
  } else if (has_pen || has_fill) { // does not antialias!
    printf("midpoint!\n");
    enableTransform(false);
    QPointF p = matrix().map(QPointF(x, y));
    if (has_pen)
      setColor(pen.color());
    else setColor(brush.color());
    drawMidpointCircle(p.x(), p.y(), d/2, has_fill);
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
      setColor(fillColor());
      QVarLengthArray<double, 4096> vertices(n*8);
      double *v = vertices.data();
      for (int i = 0; i < n; i++) {
        v[0] = x[i]; v[1] = y[i];
        v[2] = x[i] + w[i]; v[3] = y[i];
        v[4] = x[i] + w[i]; v[5] = y[i] + h[i];
        v[6] = x[i]; v[7] = y[i] + h[i];
        v += 8;
      }
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
      glDrawArrays(GL_QUADS, 0, 4*n);
      glDisableClientState(GL_VERTEX_ARRAY);
    } else QtPainter::drawRectangles(x, y, w, h, n);
}

// possible fast path for multiple colors, if stroke/fill the same:
// push colors into array, use indexed image -> alpha texture
// OOPS: apparently point sprites do not support alpha textures
/*
void OpenGLPainter::drawGlyphs(const QPainterPath &path, double *x, double *y,
                               double *size, QColor *stroke,
                               QColor *fill, int n)
{
  if (!size) {
    bool equal = true;
    int i;
    for (i = 0; i < n && equal; i++)
      equal = stroke[i] == fill[i];
    if (i == n) {
      QImage alpha = rasterizeGlyph(path).alphaChannel();
      drawGlyphs(alpha, x, y, n);
      return;
    }
  }
  QtBasePainter::drawGlyphs(path, x, y, size, stroke, fill, n);
}
*/

void OpenGLPainter::drawSomeGlyphs(const QImage &image, double *x, double *y,
                                   int n)
{
  glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
  
  glEnable(GL_TEXTURE_2D);
  /*
  GLint format = image.depth() == 32 ? GL_RGBA : GL_ALPHA;
  GLuint tex = context->bindTexture(image, GL_TEXTURE_2D, format);
  */
  GLuint tex = context->bindTexture(image);
  enableTransform();
  
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  glPointSize(image.width());
  glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
  setColor(QColor(0, 0, 0, 255));
  
  drawVertices(GL_POINTS, x, y, n);

  context->deleteTexture(tex);
  
  glPopAttrib();  
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
