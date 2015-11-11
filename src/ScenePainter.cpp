#include "ScenePainter.hpp" 
#include <QGraphicsTextItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>

/* This implements the Painter API against QGraphicsScene. For the
   most part, the mappings are clean. The transformation matrix maps
   everything into the scene. Note that we lack fine-grained
   control here, so there are scaling issues. In particular,
   non-cosmetic pens and custom dash patterns will scale with the
   view, which is undesirable.

   A key use case for this is the population of the BSP tree for fast
   spatial queries. Note that there are cases, such as glyphs,
   circles, and text, that draw untransformable items. These cover
   different regions of the data, depending on the view. One could
   maintain a scene for each view, and update the scene for each
   transformation of the view. This seems complicated.

   Consider the use cases:
   - Query/identify: click or move mouse near a point, get some
   response. R's identify() and GGobi's identify mode do a nearest
   point search, within some tolerance. There is no notion of
   overlapping the glyph; it's all about the data point.
   - Brushing: do something to points within a rectangle. Here also,
   the brush must cover the actual data point, not the glyph drawing,
   and this makes sense.

   So it seems that there should be an 'index mode' that just draws
   glyphs, circles, text, etc as single points. Also, lines need
   to be drawn with a cosmetic pen, for line brushing.
*/

using namespace Qanviz;

// mechanism for mapping from item back to graphical primitive
// -1 indicates not found
int ScenePainter::itemIndex(QGraphicsItem *item) {
  return itemToIndex.value(item) - 1;
}
    
// FIXME: we may want to return the items in order drawn, which
// would require a separate QList

#define STORE_INDEX(x) ({                              \
      QGraphicsItem *item = x;                         \
      itemToIndex.insert(item, ++itemCount);           \
      item;                                            \
    })

// draw lines
void ScenePainter::drawPolyline(double *x, double *y, int n) {
  QVector<QPointF> points(n);
  QTransform tform = transform();
  for (int i = 0; i < n; i++)
      points[i] = tform.map(QPointF(x[i], y[i]));
  QPainterPath path;
  // NOTE: polygons are not closed by default in paths
  path.addPolygon(QPolygonF(points));
  STORE_INDEX(_scene->addPath(path, pen(), brush()));
}
void ScenePainter::drawSegments(double *x0, double *y0, double *x1, double *y1,
                                int n)
{
  QTransform tform = transform();
  QPen pen = this->pen();
  for (int i = 0; i < n; i++) {
    QLineF line = QLineF(x0[i], y0[i], x1[i], y1[i]);
    STORE_INDEX(_scene->addLine(tform.map(line), pen));
  }
}

// draw path
void ScenePainter::drawPath(QPainterPath path) {
  STORE_INDEX(_scene->addPath(transform().map(path), pen(), brush()));
}

#include <limits>

// draw points (pixels)
void ScenePainter::drawPoints(double *x, double *y, int n) {
  QTransform tform = transform();
  QPen pen = this->pen();
  // seems that 0-size items are ignored, so we use min size
  float minFloat = std::numeric_limits<float>::min();
  QSizeF size = indexMode() ? QSizeF(minFloat, minFloat) : QSizeF(1, 1);
  for (int i = 0; i < n; i++) {
    QRectF rect = QRectF(QPointF(0, 0), size);
    QGraphicsItem *item = STORE_INDEX(_scene->addRect(rect, pen));
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    item->setPos(tform.map(QPointF(x[i], y[i])));
  }
}
    
// draw shapes
// NOTE: if drawing many shapes of same size, use drawGlyphs
void ScenePainter::drawRectangles(double *x, double *y, double *w, double *h,
                                  int n)
{
  QTransform tform = transform();
  QPen pen = this->pen();
  for (int i = 0; i < n; i++) {
    QPointF point = QPointF(x[i], y[i]);
    QRectF rect = tform.mapRect(QRectF(point, QSizeF(w[i], h[i])));
    if (!tform.isRotating()) {
      STORE_INDEX(_scene->addRect(tform.mapRect(rect), pen));
    }
    else STORE_INDEX(_scene->addPolygon(tform.map(QPolygonF(rect))));
  }
}

void ScenePainter::drawCircle(double x, double y, int r) {
  QRectF rect;
  if (indexMode()) {
    float minFloat = std::numeric_limits<float>::min();
    rect = QRectF(0, 0, minFloat, minFloat);
  } else rect = QRectF(-r, -r, 2*r, 2*r);
  QGraphicsItem *item = STORE_INDEX(_scene->addEllipse(rect, pen(), brush()));
  item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
  item->setPos(transform().map(QPointF(x, y)));
}

void ScenePainter::drawPolygon(double *x, double *y, int n) {
  QVector<QPointF> points(n);
  QTransform tform = transform();
  for (int i = 0; i < n; i++)
    points[i] = tform.map(QPointF(x[i], y[i]));
  STORE_INDEX(_scene->addPolygon(QPolygonF(points), pen(), brush()));
}
    
// draw text
void ScenePainter::drawText(const char * const *strs, double *x, double *y,
                            int n, Qt::Alignment flags, double rot, double hcex,
                            double vcex)
{
  if (indexMode()) {
    QVector<QRectF> rects = textExtents(strs, n, flags);
    for (int i = 0; i < n; i++) {
      QString qstr = QString::fromLocal8Bit(strs[i]);
      QPointF center = rects[i].center();
      double xi = x[i] + center.x();
      double yi = y[i] + center.y();
      drawPoints(&xi, &yi, 1);
    }
    return;
  }
  // FIXME: alignment is ignored -- translate to HTML?
  QTransform tform = transform();
  for (int i = 0; i < n; i++) {
    QString qstr = QString::fromLocal8Bit(strs[i]);
    QGraphicsItem *item = STORE_INDEX(_scene->addText(qstr, _scene->font()));
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    item->setPos(tform.map(QPointF(x[i], y[i])));
    item->rotate(-rot);
    item->scale(hcex, vcex);
  }
}
    
// image
void ScenePainter::drawImage(const QImage &image, double x, double y,
                             int sx, int sy, int sw, int sh)
{
  if (indexMode()) {
    x = (sw < 0 ? image.width() : sw) / 2.0;
    y = (sh < 0 ? image.height() : sh) / 2.0;
    drawPoints(&x, &y, 1);
    return;
  }
  QImage subImage;
  if (sw >= 0 || sh >= 0 || sx > 0 || sy > 0) {
    sw = sw < 0 ? image.width() : sw;
    sh = sh < 0 ? image.height() : sh;
    subImage = image.copy(sx, sy, sw, sh);
  } else subImage = image;
  QPixmap pixmap = QPixmap::fromImage(subImage);
  QGraphicsItem *item = STORE_INDEX(_scene->addPixmap(pixmap));
  item->setPos(transform().map(QPointF(x, y)));
  item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
}

// drawing glyphs (same path, many places)    
void ScenePainter::drawGlyphs(const QPainterPath &path, double *x, double *y,
                              double *size, QColor *stroke, QColor *fill, int n)
{
  if (indexMode()) {
    drawPoints(x, y, n);
  } else if (!rasterize()) { 
    QTransform tform = transform();
    double curSize = glyphSize();
    for (int i = 0; i < n; i++) {
      if (stroke) setStrokeColor(stroke[i]);
      if (fill) setFillColor(fill[i]);
      QGraphicsItem *item = STORE_INDEX(_scene->addPath(path, pen(), brush()));
      item->setPos(tform.map(QPointF(x[i], y[i])));
      item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      if (size) curSize = size[i];
      item->scale(curSize, curSize);
    }
  } else QtBasePainter::drawGlyphs(path, x, y, size, stroke, fill, n);
}
