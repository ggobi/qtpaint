#include "conversion.h"

  /* C++ -> R */


  /* Eventually we probably need formal classes */
  
SEXP asRRect(QRectF rect) {
  SEXP rrect = allocMatrix(REALSXP, 2, 2);
  REAL(rrect)[0] = rect.left();
  REAL(rrect)[1] = rect.right();
  REAL(rrect)[2] = rect.top();
  REAL(rrect)[3] = rect.bottom();
  return rrect;
}
SEXP asRMatrix(QMatrix matrix, bool inverted) {
  bool ok = true;
  SEXP ans = R_NilValue;
  if (inverted)
    matrix = matrix.inverted(&ok);
  if (ok) {
    ans = allocMatrix(REALSXP, 3, 2);
    REAL(ans)[0] = matrix.m11();
    REAL(ans)[1] = matrix.m21();
    REAL(ans)[2] = matrix.dx();
    REAL(ans)[3] = matrix.m12();
    REAL(ans)[4] = matrix.m22();
    REAL(ans)[5] = matrix.dy();
  }
  return ans;
}
SEXP asRPoint(QPointF point) {
  SEXP rpoint = allocVector(REALSXP, 2);
  REAL(rpoint)[0] = point.x(); REAL(rpoint)[1] = point.y();
  return rpoint;
}
  
/* R -> C++ */
QColor *asQColors(SEXP c) {
  QColor *colors = (QColor *)R_alloc(ncols(c), sizeof(QColor));
  int *rcolors = INTEGER(c);
  for (int i = 0; i < ncols(c); i++, rcolors += 4)
    colors[i] = QColor(rcolors[0], rcolors[1], rcolors[2], rcolors[3]);
  return colors;
}
QColor asQColor(SEXP c) {
  int *rcolor = INTEGER(c);
  return QColor(rcolor[0], rcolor[1], rcolor[2], rcolor[3]);
}
QMatrix asQMatrix(SEXP m) {
  double *rmatrix = REAL(m);
  return QMatrix(rmatrix[0], rmatrix[3], rmatrix[1], rmatrix[4],
                 rmatrix[2], rmatrix[5]);
}
