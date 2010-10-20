#include "Painter.hpp"

#include <QRectF>
#include <QFontMetrics>

using namespace Qanviz;

void Painter::setLimits(QRectF limits, bool combined) {
  QTransform tform;
  QRectF device = deviceRect();
  tform.scale(device.width() / limits.width(),
              -device.height() / limits.height());
  tform.translate(-limits.left(), -limits.bottom());
  setTransform(tform, combined);
}

void Painter::setTransformEnabled(bool enabled) {
  if (enabled == _transformEnabled)
    return;
  _transformEnabled = enabled;
  if (enabled)
    setTransform(savedTransform);
  else {
    QTransform ident;
    savedTransform = transform();
    setTransform(ident);
  }
}

void Painter::fontMetrics(float *ascent, float *descent) {
  QFontMetrics metrics = QFontMetrics(font());
  // ascent and descent are distances, so we use lines
  QTransform rtform = transform().inverted();
  *ascent = rtform.map(QLineF(0, 0, 0, metrics.ascent())).length();
  *descent = rtform.map(QLineF(0, 0, 0, metrics.descent())).length();
}

QVector<QRectF> Painter::textExtents(const char * const *strs, int n,
                                     Qt::Alignment alignment)
{
  QVector<QRectF> rects(n);
  QFontMetrics metrics = QFontMetrics(font());
  QTransform rtform = transform().inverted();
  for (int i = 0; i < n; i++) {
    QString qstr = QString::fromLocal8Bit(strs[i]);
    if (qstr.count('\n'))
      rects[i] = metrics.boundingRect(0, 0, 0, 0, alignment, qstr);
    // Apparently this is necessary for good vertical bounds
    // It's noted to be slow on Windows, so might want strWidth method
    else rects[i] = metrics.tightBoundingRect(qstr);
    rects[i] = rtform.mapRect(rects[i]); // map from pixels
    // no real position, so force it to sit on 0 for consistency
    rects[i].moveTop(0);
  }
  return rects;
}

void Painter::drawGlyphs(const QImage &image, double *x, double *y, int n)
{
  prepareDrawGlyphs();
  drawSomeGlyphs(image, x, y, n);
  finishDrawGlyphs();
}

void Painter::drawSomeGlyphs(const QImage &image, double *x, double *y,
                             int n)
{
  for (int i = 0; i < n; i++)
    drawImage(image, x[i], y[i]);
}
