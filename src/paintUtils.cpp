#include "paintUtils.hpp"

#include <QColor>

#include <R_ext/Arith.h>

using namespace Qanviz;

/* these two functions break primitives on NA */

#define UPDATE_ATTR(x, setter)                      \
  if (x && prev_ ## x != x[k]) {                    \
    p->setter(x[k]);                                \
    prev_ ## x = x[k];                              \
  }

void PaintUtils::drawPolylines(Painter *p, double *x, double *y, QColor *stroke,
                               int n)
{
  int j = 0, i = 0, k = 0;
  QColor prev_stroke = stroke ? stroke[0] : QColor();
  for (i = 0; i < n; i++) {
      if (!R_finite(x[i]) || !R_finite(y[i])) {
	  UPDATE_ATTR(stroke, setStrokeColor);
	  p->drawPolyline(x + j, y + j, i - j);
	  j = i + 1;
	  k++;
      }
  }
  if (n && stroke) p->setStrokeColor(stroke[k]);
  p->drawPolyline(x + j, y + j, n - j);
}

void PaintUtils::drawPolygons(Painter *p, double *x, double *y, QColor *stroke,
                              QColor *fill, int n)
{
  int j = 0, i = 0, k = 0;
  QColor prev_stroke, prev_fill;
  if (n) {
    if (stroke) {
      p->setStrokeColor(stroke[0]);
      prev_stroke = stroke[0];
    }
    if (fill) {
      p->setFillColor(fill[0]);
      prev_fill = fill[0];
    }
  }
  for (i = 0; i < n; i++) {
    if (!R_finite(x[i]) || !R_finite(y[i])) {
      UPDATE_ATTR(stroke, setStrokeColor);
      UPDATE_ATTR(fill, setFillColor);
      p->drawPolygon(x + j, y + j, i - j);
      j = i + 1;
      k++;
    }
  }
  if (n) {
    if (stroke) p->setStrokeColor(stroke[k]);
    if (fill) p->setFillColor(fill[k]);
  }
  p->drawPolygon(x + j, y + j, i - j);
}
