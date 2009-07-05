#include "Painter.hpp"
#include "paintUtils.hpp"

#include <qtbase.h>

using namespace QViz;

#define OPTIONAL(x, cast) x == R_NilValue ? NULL : cast(x)
#define COLOR(x) OPTIONAL(x, asQColors)

#define DRAW_PRIMITIVES(op, n) ({                                       \
  QColor *stroke = COLOR(rstroke);                                      \
  int j = 0, i = n;                                                     \
  if (n && stroke) {                                                    \
    QColor prevStroke = stroke[i];                                      \
    for (i = 0; i < n; i++) {                                           \
      if (stroke[i] != prevStroke) {                                    \
        p->setStrokeColor(stroke[i-1]);                                 \
        prevStroke = stroke[i-1];                                       \
        p->op;                                                          \
        j = i;                                                          \
      }                                                                 \
    }                                                                   \
    p->setStrokeColor(stroke[i-1]);                                     \
  }                                                                     \
  p->op;                                                                \
})

extern "C" {

#define PAINTER_P() Painter *p = unwrapPointer(rp, Painter)

  // retrieve transformation (when pixels matter)
  SEXP qt_qmatrix_Painter(SEXP rp, SEXP rinverted) {
    PAINTER_P();
    return asRMatrix(p->matrix(), asLogical(rinverted));
  }

  SEXP qt_qsetMatrix_Painter(SEXP rp, SEXP rmatrix) {
    PAINTER_P();
    p->setMatrix(asQMatrix(rmatrix));
    return rp;
  }

  SEXP qt_qsetMatrixEnabled_Painter(SEXP rp, SEXP renabled) {
    PAINTER_P();
    p->setMatrixEnabled(asLogical(renabled));
    return rp;
  }
  
  // colors
  SEXP qt_qsetHasStroke_Painter(SEXP rp, SEXP rstroke) {
    PAINTER_P();
    p->setHasStroke(asLogical(rstroke));
    return rp;
  }
  SEXP qt_qsetHasFill_Painter(SEXP rp, SEXP rfill) {
    PAINTER_P();
    p->setHasFill(asLogical(rfill));
    return rp;
  }
  SEXP qt_qsetStrokeColor_Painter(SEXP rp, SEXP rcolor)
  {
    PAINTER_P();
    p->setStrokeColor(asQColor(rcolor));
    return rp;
  }
  SEXP qt_qsetFillColor_Painter(SEXP rp, SEXP rcolor)
  {
    PAINTER_P();
    p->setFillColor(asQColor(rcolor));
    return rp;
  }

  // font
  SEXP qt_qsetFont_Painter(SEXP rp, SEXP family, SEXP ps, SEXP weight, SEXP italic)
  {
    PAINTER_P();
    p->setFont(QFont(sexp2qstring(family), asInteger(ps), asInteger(weight),
                     asLogical(italic)));
    return rp;
  }
  SEXP qt_qfontMetrics_Painter(SEXP rp) {
    PAINTER_P();
    float ascent, descent;
    p->fontMetrics(&ascent, &descent);
    SEXP ans;
    PROTECT(ans = allocVector(REALSXP, 2));
    REAL(ans)[0] = ascent;
    REAL(ans)[1] = descent;
    UNPROTECT(1);
    return ans;
  }
  SEXP qt_qtextExtents_Painter(SEXP rp, SEXP rstrs) {
    PAINTER_P();
    int n = length(rstrs);
    QVector<QRectF> rects = p->textExtents(asStringArray(rstrs), n);
    SEXP ans = allocMatrix(REALSXP, n, 4);
    for (int i = 0; i < n; i++) {
      REAL(ans)[i] = rects[i].left();
      REAL(ans)[i+n] = rects[i].top();
      REAL(ans)[i+2*n] = rects[i].right();
      REAL(ans)[i+3*n] = rects[i].bottom();
    }
    return ans;
  }
  
  // line aesthetics
  SEXP qt_qsetLineWidth_Painter(SEXP rp, SEXP rwidth) {
    PAINTER_P();
    p->setLineWidth(asInteger(rwidth));
    return rp;
  }
  SEXP qt_qsetDashes_Painter(SEXP rp, SEXP rdashes) {
    PAINTER_P();
    p->setDashes(REAL(rdashes), length(rdashes));
    return rp;
  }
  
  // antialiasing option
  SEXP qt_qsetAntialias_Painter(SEXP rp, SEXP rantialias) {
    PAINTER_P();
    p->setAntialias(asLogical(rantialias));
    return rp;
  }
  
  // draw lines
  SEXP qt_qdrawPolyline_Painter(SEXP rp, SEXP rx, SEXP ry, SEXP rstroke)
  {
    PAINTER_P();
    PaintUtils::drawPolylines(p, REAL(rx), REAL(ry), COLOR(rstroke),
                              length(rx));
    return rp;
  }
  SEXP qt_qdrawSegments_Painter(SEXP rp, SEXP rx0, SEXP ry0, SEXP rx1, SEXP ry1,
                                SEXP rstroke)
  {
    PAINTER_P();
    DRAW_PRIMITIVES(drawSegments(REAL(rx0) + j, REAL(ry0) + j, REAL(rx1) + j,
                                 REAL(ry1) + j, i - j), length(rx0));
    return rp;
  }
   
  // draw points (pixels)
  SEXP qt_qdrawPoints_Painter(SEXP rp, SEXP rx, SEXP ry, SEXP rstroke) {
    PAINTER_P();
    DRAW_PRIMITIVES(drawPoints(REAL(rx) + j, REAL(ry) + j, i - j), length(rx));
    return rp;
  }
  
  // draw shapes
  // NOTE: if drawing many shapes of same size, use drawGlyphs
  // NOTE: this follows the top left, width, height convention
  SEXP qt_qdrawRectangles_Painter(SEXP rp, SEXP rx, SEXP ry, SEXP rw, SEXP rh,
                              SEXP rstroke, SEXP rfill)
  {
    PAINTER_P();
    QColor *stroke = COLOR(rstroke);
    QColor *fill = COLOR(rfill);
    int n = length(rx);
    int j = 0, i = n;
    if (n && (stroke || fill)) {
      QColor prevStroke, prevFill;
      if (stroke) {
        p->setStrokeColor(stroke[0]);
        prevStroke = stroke[0];
      }
      if (fill) {
        p->setFillColor(fill[0]);
        prevFill = fill[0];
      }
      for (i = 0; i < n; i++) {
        bool changed = false;
        if (stroke && stroke[i] != prevStroke) {
          p->setStrokeColor(stroke[i-1]);
          prevStroke = stroke[i-1];
          changed = true;
        }
        if (fill && fill[i] != prevFill) {
          p->setFillColor(fill[i-1]);
          prevFill = fill[i-1];
          changed = true;
        }                                                                 
        if (changed) {
          p->drawRectangles(REAL(rx) + j, REAL(ry) + j, REAL(rw) + j,
                            REAL(rh) + j, i - j);
          j = i;
        }
      }
      if (stroke) p->setStrokeColor(stroke[i-1]);
      if (fill) p->setFillColor(fill[i-1]);
    }
    p->drawRectangles(REAL(rx) + j, REAL(ry) + j, REAL(rw) + j,
                      REAL(rh) + j, i - j);
    return rp;
  }
  SEXP qt_qdrawCircle_Painter(SEXP rp, SEXP rx, SEXP ry, SEXP rr,
                          SEXP rstroke, SEXP rfill)
  {
    PAINTER_P();
    int i, n = length(rx);
    QColor *stroke = COLOR(rstroke);
    QColor *fill = COLOR(rfill);
    QColor prevStroke, prevFill;
    double *x = REAL(rx);
    double *y = REAL(ry);
    int *r = INTEGER(rr);
    if (stroke && n) {
      p->setStrokeColor(stroke[0]);
      prevStroke = stroke[0];
    }
    if (fill && n) {
      p->setFillColor(fill[0]);
      prevFill = fill[0];
    }
    for (i = 0; i < n; i++) {
      if (stroke && stroke[i] != prevStroke) {
        p->setStrokeColor(stroke[i]);
        prevStroke = stroke[i];
      }
      if (fill && fill[i] != prevFill) {
        p->setFillColor(fill[i]);
        prevFill = fill[i];
      }
      p->drawCircle(x[i], y[i], r[i]);
    }
    return rp;
  }
  SEXP qt_qdrawPolygon_Painter(SEXP rp, SEXP rx, SEXP ry, SEXP rstroke,
                               SEXP rfill)
  {
    PAINTER_P();
    PaintUtils::drawPolygons(p, REAL(rx), REAL(ry), COLOR(rstroke),
                             COLOR(rfill), length(rx));
    return rp;
  }
  
  // draw text
  SEXP qt_qdrawText_Painter(SEXP rp, SEXP rstrs, SEXP rx, SEXP ry, SEXP rflags,
                        SEXP rrot)
  {
    PAINTER_P();
    p->drawText(asStringArray(rstrs), REAL(rx), REAL(ry), length(rx),
                (Qt::Alignment)asInteger(rflags), asReal(rrot));
    return rp;
  }

  // drawing glyphs (same path, many places)
  SEXP qt_qdrawGlyphs_Painter(SEXP rp, SEXP rpath, SEXP rx, SEXP ry, SEXP rsize,
                          SEXP rstroke, SEXP rfill)
  {
    PAINTER_P();
    p->drawGlyphs(*unwrapPointer(rpath, QPainterPath), REAL(rx), REAL(ry),
                  OPTIONAL(rsize, REAL), COLOR(rstroke), COLOR(rfill),
                  length(rx));
    return rp;
  }
  
  // This should go away, after we have ways to construct QImage
  SEXP qt_qdrawImageRaw_Painter(SEXP rp, SEXP rcol, SEXP rwidth, SEXP rheight,
                            SEXP rx, SEXP ry)
  {
    PAINTER_P();
    QImage image(RAW(rcol), asInteger(rwidth), asInteger(rheight),
                 QImage::Format_ARGB32);
    p->drawGlyphs(image, REAL(rx), REAL(ry), length(rx));
    return rp;
  }

#define QPAINTERPATH_P() QPainterPath *p = unwrapPointer(rp, QPainterPath)

  void finalizePainterPath(SEXP rpath) {
    QPainterPath *path = unwrapPointer(rpath, QPainterPath);
    R_ClearExternalPtr(rpath);
    delete path;
  }
  
  SEXP qt_qpath(void) {
    QList<QString> classes;
    classes.append("QPainterPath");
    QPainterPath *path = new QPainterPath();
    return wrapPointer(path, classes, finalizePainterPath);
  }

  SEXP qt_qaddCircle_QPainterPath(SEXP rp, SEXP rx, SEXP ry, SEXP rr) {
    QPAINTERPATH_P();
    p->addEllipse(QPointF(asReal(rx), asReal(ry)), asReal(rr), asReal(rr));
    return rp;
  }

  SEXP qt_qaddRect_QPainterPath(SEXP rp, SEXP rx, SEXP ry, SEXP dx, SEXP dy) {
    QPAINTERPATH_P();
    p->addRect(asReal(rx), asReal(ry), asReal(dx), asReal(dy));
    return rp;
  }

  SEXP qt_qaddPolygon_QPainterPath(SEXP rp, SEXP rx, SEXP ry) {
    QPAINTERPATH_P();
    QPolygonF polygon;
    double *x = REAL(rx);
    double *y = REAL(ry);
    for (int i = 0; i < length(rx); i++)
      polygon << QPointF(x[i], y[i]);
    p->addPolygon(polygon);
    return rp;
  }

  SEXP qt_qaddText_QPainterPath(SEXP rp, SEXP rtext, SEXP rx, SEXP ry,
                                SEXP rfamily, SEXP rps, SEXP rweight,
                                SEXP ritalic)
  {
    QPAINTERPATH_P();
    QFont font(sexp2qstring(rfamily), asInteger(rps), asInteger(rweight),
               asLogical(ritalic));
    p->addText(asReal(rx), asReal(ry), font, sexp2qstring(rtext));
    return rp;
  }

  SEXP qt_qaddLine_QPainterPath(SEXP rp, SEXP rx0, SEXP ry0, SEXP rx1,
                                SEXP ry1)
  {
    QPAINTERPATH_P();
    p->moveTo(asReal(rx0), asReal(ry0));
    p->lineTo(asReal(rx1), asReal(ry1));
    return rp;
  }

}
