#include <QRectF>
#include <QMatrix>
#include <QString>
#include <QColor>
#include <qtbase.h>

#include <Rinternals.h>

extern "C" {
  SEXP asRRect(QRectF rect);
  SEXP asRMatrix(QMatrix matrix, bool inverted);
  SEXP asRPoint(QPointF point);
  
  QColor *asQColors(SEXP c);
  QColor asQColor(SEXP c);
}
