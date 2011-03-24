#include "QtBasePainter.hpp"
#include <QPainter>

using namespace Qanviz;

void QtBasePainter::setHasStroke(bool enabled) {
  if (enabled == hasStroke())
    return;
  QPen pen = this->pen();
  if (enabled) {
    if (dashPattern.isEmpty())
      pen.setStyle(Qt::SolidLine);
    else pen.setDashPattern(dashPattern); // restore dash pattern
  } else {
    pen.setStyle(Qt::NoPen); // clears dash pattern
  }
  setPen(pen);
}

// FIXME: disabling the stroke/fill when alpha is passed as zero is a
// bad idea, as it introduces inconsistency: using setHasStroke/Fill,
// one can disable drawing without setting the alpha to zero.
void QtBasePainter::setStrokeColor(QColor color) {
  setHasStroke(color.alpha());
  QPen pen = this->pen();
  pen.setColor(color);
  setPen(pen);
}

void QtBasePainter::setHasFill(bool enabled) {
  if (enabled == hasFill())
    return;
  QBrush brush = this->brush();
  if (enabled)
    brush.setStyle(Qt::SolidPattern);
  else brush.setStyle(Qt::NoBrush);
  setBrush(brush);
}

void QtBasePainter::setFillColor(QColor color) {
  setHasFill(color.alpha());
  QBrush brush = this->brush();
  brush.setColor(color);
  setBrush(brush);
}


// line aesthetics  
void QtBasePainter::setLineWidth(uint width) {
  QPen pen = this->pen();
  int oldWidth = pen.width() ? pen.width() : 1;
  if (width == 1)
    width = 0; // take cosmetic brush fast path
  pen.setWidth(width);
  // NOTE: can't use world transform for non-cosmetic brush
  if (!dashPattern.isEmpty()) { // update dashes for the line width
    for (int i = 0; i < dashPattern.size(); i++)
      dashPattern[i] = dashPattern[i] * oldWidth / width;
    if (pen.style() != Qt::NoPen)
      pen.setDashPattern(dashPattern);
  }
  setPen(pen);
}

void QtBasePainter::setDashes(double *dashes, int n) {
  QPen pen = this->pen();
  dashPattern.clear();
  if (dashes) {
    // the dependency of the dash pattern on line width is very strange
    int width = pen.width() ? pen.width() : 1;
    for (int i = 0; i < n; i++)
      dashPattern << dashes[i] / width;
    if (pen.style() != Qt::NoPen)
      pen.setDashPattern(dashPattern);
  } else if (pen.style() != Qt::NoPen)
    pen.setStyle(Qt::SolidLine); // NULL clears dash pattern
  setPen(pen);
}

void QtBasePainter::setLineCap(Qt::PenCapStyle cap) {
  QPen pen = this->pen();
  pen.setCapStyle(cap);
  setPen(pen);
}
void QtBasePainter::setLineJoin(Qt::PenJoinStyle join) {
  QPen pen = this->pen();
  pen.setJoinStyle(join);
  setPen(pen);
}

void QtBasePainter::setMiterLimit(double limit) {
  QPen pen = this->pen();
  pen.setMiterLimit(limit);
  setPen(pen);
}

QImage QtBasePainter::rasterizeGlyph(const QPainterPath &path) {
  QRectF bounds = path.boundingRect();
  double size = glyphSize();
  int lw = lineWidth() - 1;
  if (lw < 0) lw = 0;
  QImage image((bounds.width() + 2 + lw)*size, (bounds.height() + 2 + lw)*size,
               QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter imagePainter(&image);
  int boundsAdj = lw / 2.0 + 1.5;
  imagePainter.scale(size, size);
  imagePainter.translate(-bounds.x() + boundsAdj, -bounds.y() + boundsAdj);
  // all glyphs are antialiased
  imagePainter.setRenderHint(QPainter::Antialiasing);
  imagePainter.setBrush(brush());
  imagePainter.setPen(pen());
  imagePainter.setFont(font());
  imagePainter.setOpacity(opacity());
  imagePainter.drawPath(path);
  return image;
}

void QtBasePainter::drawGlyphs(const QPainterPath &path, double *x, double *y,
                               double *size, QColor *stroke, QColor *fill,
                               int n)
{
  // TRYME: non-antialiased fast paths: bitmaps, two-color indexed
  prepareDrawGlyphs();
  int j = 0, i = n;
  if (n && (stroke || fill || size)) {
    QColor prevStroke, prevFill;
    double prevSize = 0;
    if (stroke) {
      setStrokeColor(stroke[0]);
      prevStroke = stroke[0];
    }
    if (fill) {
      setFillColor(fill[0]);
      prevFill = fill[0];
    }
    if (size) {
      setGlyphSize(size[0]);
      prevSize = size[0];
    }
    for (i = 0; i < n; i++) {
      bool changed = false;
      if (stroke && stroke[i] != prevStroke) {
        setStrokeColor(stroke[i-1]);                              
        prevStroke = stroke[i];
        changed = true;
      }
      if (fill && fill[i] != prevFill) {
        setFillColor(fill[i-1]);
        prevFill = fill[i];
        changed = true;
      }
      if (size && size[i] != prevSize) {
        setGlyphSize(size[i-1]);
        prevSize = size[i];
        changed = true;
      }
      if (changed) {
        drawSomeGlyphs(rasterizeGlyph(path), x + j, y + j, i - j);
        j = i;
      }
    }
    if (stroke) setStrokeColor(stroke[i-1]);                              
    if (fill) setFillColor(fill[i-1]);
    if (size) setGlyphSize(size[i-1]);
  }
  drawSomeGlyphs(rasterizeGlyph(path), x + j, y + j, i - j);
  finishDrawGlyphs();
}
