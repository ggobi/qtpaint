#include <QColor>
#include <Rinternals.h>

/* colors */
QColor asQColor(SEXP c);
QColor *asQColors(SEXP c);

/* string arrays */
const char ** asStringArray(SEXP s_strs);
SEXP asRStringArray(const char * const * strs);
