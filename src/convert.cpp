#include "convert.hpp"

/* Color conversion, col2rgb() matrix -> QColor */
/* The qtbase package provides explicit coercion routines for this,
   but qtpaint does it implicitly in C for performance reasons. */

QColor asQColor(SEXP c) {
  int *rcolor = INTEGER(c);
  return QColor(rcolor[0], rcolor[1], rcolor[2], rcolor[3]);
}

QColor *asQColors(SEXP c) {
  int ncolors = length(c) / 4;
  QColor *colors = (QColor *)R_alloc(ncolors, sizeof(QColor));
  int *rcolors = INTEGER(c);
  for (int i = 0; i < ncolors; i++, rcolors += 4)
    colors[i] = QColor(rcolors[0], rcolors[1], rcolors[2], rcolors[3]);
  return colors;
}

/* string arrays (more of a utility) */

const char ** asStringArray(SEXP s_strs) {
  const char **strs = (const char **)R_alloc(length(s_strs), sizeof(char *));
  for (int i = 0; i < length(s_strs); i++)
    strs[i] = CHAR(STRING_ELT(s_strs, i));
  return strs;
}

SEXP asRStringArray(const char * const * strs) {
  SEXP ans;
  int n = 0;
  while(strs[n])
    n++;
  PROTECT(ans = allocVector(STRSXP, n));
  for (int i = 0; i < n; i++)
    SET_STRING_ELT(ans, i, mkChar(strs[i]));
  UNPROTECT(1);
  return ans;
}
