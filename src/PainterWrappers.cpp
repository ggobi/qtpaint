#include <qtbase.h>

#include <QFont>

#include "Painter.hpp"
#include "paintUtils.hpp"
#include "convert.hpp"

using namespace Qanviz;

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
        prevStroke = stroke[i];                                         \
        p->op;                                                          \
        j = i;                                                          \
      }                                                                 \
    }                                                                   \
    p->setStrokeColor(stroke[i-1]);                                     \
  }                                                                     \
  p->op;                                                                \
})

extern "C" {

  /* We do not use Smoke for these bindings, since performance is so
     important. We could work-around issues like the double array (use
     QList<float>), and the Smoke method call overhead probably does
     not matter. But there are issues:

     - We special-case QColor conversion, so that we do not need to
       create a QList<QColor> in R. We could special-case this
       conversion in qtbase, but that would be ugly. Idea: introduce
       special class for this, with special converter.

     - Special-casing of the font metrics and text extents. Would need
       special classes/conversions for this, as well.

     - We currently do a lot of vectorization in these wrappers. This
       really should be pushed down to Painter, for consistency with
       drawGlyphs, but it would take some work.

     - The R wrappers put a more R-like interface on things. Would
       that have to stay? Would Painter need an adaptor?
     
     The question is whether the above work outweighs the benefits,
     which are much fewer than with e.g. Layer, as Painter is just a
     QObject. The main gain would be the deletion of these manual
     bindings, which are already well tested, but incomplete.

     Conclusion: let's hold off for now.
  */
  
#define PAINTER_P() Painter *p = unwrapPointer(rp, Painter)

  // retrieve transformation (when pixels matter)
  SEXP qt_qtransform_Painter(SEXP rp) {
    PAINTER_P();
    return wrapSmokeCopy(p->transform(), QTransform);
  }

  SEXP qt_qsetTransform_Painter(SEXP rp, SEXP rtform) {
    PAINTER_P();
    p->setTransform(*unwrapSmoke(rtform, QTransform));
    return rp;
  }

  SEXP qt_qsetTransformEnabled_Painter(SEXP rp, SEXP renabled) {
    PAINTER_P();
    p->setTransformEnabled(asLogical(renabled));
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
  SEXP qt_qsetFont_Painter(SEXP rp, SEXP rfont)
  {
    PAINTER_P();
    p->setFont(*unwrapSmoke(rfont, QFont));
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

  // glyph size
  SEXP qt_qsetGlyphExpansion_Painter(SEXP rp, SEXP rsize) {
    PAINTER_P();
    p->setGlyphSize(asReal(rsize));
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

  // draw paths
  SEXP qt_qdrawPath_Painter(SEXP rp, SEXP rpath, SEXP rstroke, SEXP rfill)
  {
    PAINTER_P();
    int i, n = length(rpath);
    QColor *stroke = COLOR(rstroke);
    QColor *fill = COLOR(rfill);
    QColor prevStroke, prevFill;
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
      p->drawPath(*unwrapSmoke(VECTOR_ELT(rpath, i), QPainterPath));
    }
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
          prevStroke = stroke[i];
          changed = true;
        }
        if (fill && fill[i] != prevFill) {
          p->setFillColor(fill[i-1]);
          prevFill = fill[i];
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
                            SEXP rrot, SEXP rcolor, SEXP rhcex, SEXP rvcex)
  {
    PAINTER_P();
    int i, n = length(rx), j = 0;
    QColor *color = COLOR(rcolor);
    QColor prevColor;
    double *rot = REAL(rrot), prevRot;
    double *x = REAL(rx);
    double *y = REAL(ry);
    double *hcex = REAL(rhcex), *vcex = REAL(rvcex), prevHCex, prevVCex;
    const char * const *strs = asStringArray(rstrs);
    Qt::Alignment flags = (Qt::Alignment)asInteger(rflags);
    if (n) {
      prevRot = rot[0];
      prevHCex = hcex[0];
      prevVCex = vcex[0];
      if (color) {
        p->setStrokeColor(color[0]);
        prevColor = color[0];
      }
    }
    for (i = 0; i < n; i++) {
      bool changed = false;
      if (color && color[i] != prevColor) {
        p->setStrokeColor(color[i-1]);
        prevColor = color[i];
        changed = true;
      }
      if (rot[i] != prevRot) {
        prevRot = rot[i];
        changed = true;
      }
      if (hcex[i] != prevHCex) {
        prevHCex = hcex[i];
        changed = true;
      }
      if (vcex[i] != prevVCex) {
        prevVCex = vcex[i];
        changed = true;
      }
      if (changed) {
        p->drawText(strs + j, x + j, y + j, i - j, flags, rot[j], hcex[j],
                    vcex[j]);
        j = i;
      }
    }
    if (color)
      p->setStrokeColor(color[i-1]);
    p->drawText(strs + j, x + j, y + j, i - j, flags, rot[j], hcex[j], vcex[j]);
    return rp;
  }

  // drawing glyphs (same path, many places)
  SEXP qt_qdrawGlyphs_Painter(SEXP rp, SEXP rpath, SEXP rx, SEXP ry, SEXP rsize,
                              SEXP rstroke, SEXP rfill)
  {
    PAINTER_P();
    p->drawGlyphs(*unwrapSmoke(rpath, QPainterPath), REAL(rx), REAL(ry),
                  OPTIONAL(rsize, REAL), COLOR(rstroke), COLOR(rfill),
                  length(rx));
    return rp;
  }

  SEXP qt_qdrawImage_Painter(SEXP rp, SEXP rimage, SEXP rx, SEXP ry)
  {
    PAINTER_P();
    p->drawGlyphs(*unwrapSmoke(rimage, QImage), REAL(rx), REAL(ry),
                  length(rx));
    return rp;
  }
}
