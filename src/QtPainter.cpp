#include "QtPainter.hpp"
#include <QVarLengthArray>

using namespace Qanviz;

// font
void QtPainter::setFont(QFont font) {
  painter->setFont(font);
}

// struct equivalent of QPointF, to avoid QPointF() overhead
// Qt really needs these as structs, but perhaps abusing C++ is better?
struct PointF {
  qreal x;
  qreal y;
};

// draw lines
void QtPainter::drawPolyline(double *x, double *y, int n) {
  Q_ASSERT(sizeof(PointF) == sizeof(QPointF));
  QVarLengthArray<PointF, 4096> p(n);
  for (int i = 0; i < n; i++) {
    p[i].x = x[i];
    p[i].y = y[i];
  }
  QPointF *points = (QPointF *)p.data();
  if (!simplePen()) {
    QTransform tform = transform();   
    for (int i = 0; i < n; i++)
      points[i] = tform.map(points[i]);
  }
  painter->setWorldMatrixEnabled(simplePen());
  painter->drawPolyline(points, n);
}

struct LineF {
  PointF a;
  PointF b;
};

void QtPainter::drawSegments(double *x0, double *y0, double *x1, double *y1,
                             int n)
{
  Q_ASSERT(sizeof(PointF) == sizeof(QPointF));
  Q_ASSERT(sizeof(LineF) == sizeof(QLineF));
  QVarLengthArray<LineF, 4096> l(n);
  for (int i = 0; i < n; i++) {
    l[i].a.x = x0[i]; l[i].a.y = y0[i];
    l[i].b.x = x1[i]; l[i].b.y = y1[i];
  }
  QLineF *lines = (QLineF *)l.data();
  if (!simplePen()) {
    QTransform tform = transform();
    for (int i = 0; i < n; i++)
      lines[i] = tform.map(lines[i]);
  }
  painter->setWorldMatrixEnabled(simplePen());
  painter->drawLines(lines, n);
}

// draw path
void QtPainter::drawPath(QPainterPath path) {
  if (!simplePen())
    path = transform().map(path);
  painter->setWorldMatrixEnabled(simplePen());
  painter->drawPath(path);  
}

// draw points (pixels)
void QtPainter::drawPoints(double *x, double *y, int n) {
  Q_ASSERT(sizeof(PointF) == sizeof(QPointF));
  QVarLengthArray<PointF, 4096> p(n);
  for (int i = 0; i < n; i++) {
    p[i].x = x[i];
    p[i].y = y[i];
  }
  painter->setWorldMatrixEnabled(true);
  painter->drawPoints((QPointF *)p.data(), n);
}

struct RectF {
  qreal x;
  qreal y;
  qreal w;
  qreal h;
};

// draw shapes
void QtPainter::drawRectangles(double *x, double *y, double *w, double *h,
                               int n)
{
  // FIXME: this would be much faster, at least in OpenGL, if we
  // aligned the rectangle to pixels (ie toAlignedRect, but probably
  // faster to do floor/ceiling here).. but this might introduce
  // artifacts
  Q_ASSERT(sizeof(RectF) == sizeof(RectF));
  QVarLengthArray<RectF, 4096> r(n);
  for (int i = 0; i < n; i++) {
    r[i].x = x[i];
    r[i].y = y[i];
    r[i].w = w[i];
    r[i].h = h[i];
  }
  QRectF *rects = (QRectF *)r.data();
  if (!simplePen()) {
    QTransform tform = transform();
    for (int i = 0; i < n; i++)
      rects[i] = tform.mapRect(rects[i]);
  }
  painter->setWorldMatrixEnabled(simplePen());
  painter->drawRects(rects, n);
}

// if drawing many circles of same size, use drawGlyphs
void QtPainter::drawCircle(double x, double y, int r) {
  QTransform tform = transform();
  painter->setWorldMatrixEnabled(false);
  QPointF center = tform.map(QPointF(x, y));
  painter->drawEllipse(center, r, r);
}

void QtPainter::drawPolygon(double *x, double *y, int n) {
  QVarLengthArray<PointF> p(n);
  for (int i = 0; i < n; i++) {
    p[i].x = x[i];
    p[i].y = y[i];
  }
  QPointF *points = (QPointF *)p.data();
  if (!simplePen()) {
    QTransform tform = transform();
    for (int i = 0; i < n; i++)
      points[i] = tform.map(points[i]);
  }
  painter->setWorldMatrixEnabled(simplePen());
  painter->drawPolygon(points, n);
}

void QtPainter::drawText(const char * const *strs, double *x, double *y,
                         int n, Qt::Alignment flags, double rot, double hcex,
                         double vcex)
{
  QTransform tform = transform();
  for (int i = 0; i < n; i++) {
    QString qstr;
    QRectF brect;
    QPointF origin = tform.map(QPointF(x[i], y[i]));
    //printf("'%s' origin: %f %f (%f %f)\n", strs[i], origin.x(), origin.y(),
    //       x[i], y[i]);
    qstr = QString::fromLocal8Bit(strs[i]);
    painter->resetTransform();
    painter->translate(origin);
    painter->rotate(-rot);
    painter->scale(hcex, vcex);
    QRectF rect = painter->boundingRect(0, 0, 0, 0, flags, qstr);
    //printf("bounds: %f %f %f %f\n", rect.left(), rect.top(), rect.right(),
    //       rect.bottom());
    painter->drawText(rect, flags, qstr);
    //painter->drawText(QPointF(0, 0), qstr); // at baseline
  }
  setTransform(tform);
}

// image
void QtPainter::drawImage(const QImage &image, double x, double y,
                          int sx, int sy, int sw, int sh)
{
  // assume the image is in device space
  QTransform tform = transform();
  painter->setWorldMatrixEnabled(false);
  painter->drawImage(tform.map(QPointF(x, y)), image, QRectF(sx, sy, sw, sh));
}


void QtPainter::prepareDrawGlyphs(void) {
  QRectF dr = deviceRect();
  glyphBuffer = new QImage(dr.width(), dr.height(),
                           QImage::Format_ARGB32_Premultiplied);
  glyphBuffer->fill(0);
}

void QtPainter::drawSomeGlyphs(const QImage &image, double *x, double *y, int n)
{
  QPainter bufferPainter(glyphBuffer);
  QRectF source(0, 0, image.width(), image.height());
  QTransform tform = transform();
  bufferPainter.translate(-image.width() / 2, -image.height() / 2);
  for (int k = 0; k < n; k++) {
    bufferPainter.drawImage(tform.map(QPointF(x[k], y[k])), image, source);
  }  
}

void QtPainter::finishDrawGlyphs(void) {
  painter->setWorldMatrixEnabled(false);
  qreal prevOpacity = painter->opacity();
  // The glyph already incorporates opacity, so we blit with 1.0 here
  painter->setOpacity(1.0);
  painter->drawImage(0, 0, *glyphBuffer);
  painter->setOpacity(prevOpacity);
  delete glyphBuffer;
  glyphBuffer = NULL;
}
