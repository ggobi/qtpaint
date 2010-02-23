#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include "Painter.hpp"

namespace Qanviz {
  namespace PaintUtils {
    void drawPolylines(Painter *p, double *x, double *y, QColor *stroke, int n);
    void drawPolygons(Painter *p, double *x, double *y, QColor *stroke,
                      QColor *fill, int n);
  }
}

#endif
