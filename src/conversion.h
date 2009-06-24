#include <QRectF>
#include <QMatrix>
#include <QString>
#include <QColor>
#include <qtbase.h>

#include <Rinternals.h>

SEXP asRRect(QRectF rect);
SEXP asRMatrix(QMatrix matrix, bool inverted);
SEXP asRPoint(QPointF point);

QColor *asQColors(SEXP c);
QColor asQColor(SEXP c);
QMatrix asQMatrix(SEXP m);

QMatrix flipYMatrix(qreal max);

// at some point might want to do something special
#define wrapQGraphicsScene wrapQObject
