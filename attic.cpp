


SEXP
showWidget(SEXP extp)
{
    asQWidget(extp)->show();
    return R_NilValue;
}

SEXP
closeWidget(SEXP extp)
{
    asQWidget(extp)->close();
    return R_NilValue;
}

SEXP
unparentWidget(SEXP extp)
{
    asQWidget(extp)->setParent(0);
    return R_NilValue;
}


SEXP
finalizeWidget(SEXP extp)
{
    delete asQWidget(extp);
    R_ClearExternalPtr(extp);
    return R_NilValue;
}


SEXP
renderWidget(SEXP extp, SEXP file)
{
    QWidget *w = asQWidget(extp);
    if (w) {
	QPrinter printer(QPrinter::ScreenResolution);
	if (file != R_NilValue) {
	    printer.setOutputFileName(sexp2qstring(file));
	    printer.setPaperSize(QSizeF::QSizeF(7.0, 7.0), QPrinter::Inch);
	}
	else {
	    QPrintDialog *printDialog = new QPrintDialog(&printer, 0);
	    if (printDialog->exec() != QDialog::Accepted) return R_NilValue;
	    printer.setPaperSize(QPrinter::A4);
	}
	QPainter painter(&printer);
	QRect rect = printer.pageRect();
	qreal pscale = 1.0 * rect.width() / w->width();
	if (pscale > 1.0 * rect.height() / w->height())
	    pscale = 1.0 * rect.height() / w->height();
	painter.scale(pscale, pscale);
	// painter.setRenderHint(QPainter::Antialiasing, false);
	w->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }
    return R_NilValue;
}


SEXP
renderViewWidget(SEXP extp)
{
    QGraphicsView *view = qobject_cast<QGraphicsView *>((QObject *) R_ExternalPtrAddr(extp));
    if (view) {
	QPrinter printer(QPrinter::ScreenResolution);
	QPrintDialog *printDialog = new QPrintDialog(&printer, 0);
	if (printDialog->exec() == QDialog::Accepted) {
	    printer.setPageSize(QPrinter::A4);
	    QPainter painter(&printer);
	    view->render(&painter);
	}
    }
    else {
	warning("Cannot render as a view: invalid object");
    }
    return R_NilValue;
}


SEXP
renderWidgetToPixmap(SEXP extp, SEXP file)
{
    QWidget *w = asQWidget(extp);

//     QPixmap pixmap(w->size());
//     //w->render(&pixmap);
//     w->render(&pixmap, QPoint(), QRegion(), QWidget::DrawChildren);

    QPixmap pixmap = QPixmap::grabWidget(w);

    if (file == R_NilValue) {
	QLabel *label = new QLabel(0);
	label->setPixmap(pixmap);
	label->show();
    }
    else {
	if (!pixmap.save(sexp2qstring(file))) {
	    warning("Saving pixmap to file failed");
	};
    }
    return R_NilValue;
}


SEXP
renderWidgetToSVG(SEXP extp, SEXP file)
{
    if (file == R_NilValue) return R_NilValue;
    QWidget *w = asQWidget(extp);
    if (w) {
	QSvgGenerator svggen;
	svggen.setFileName(sexp2qstring(file));
	QPainter painter(&svggen);
	w->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }
    return R_NilValue;
}




#include <QWebView>

#ifndef WIN32
#include <unistd.h>
#endif

#include <R.h>
#include <Rinternals.h>

#define qextp ((QWebView *) R_ExternalPtrAddr(extp))

// R-level interface to constructor


extern "C" {

SEXP
newBrowserWidget(SEXP url)
{
    SEXP ans;    
    QWebView *view = new QWebView(0);
    view->setUrl(QString(CHAR(asChar(url))));
    ans = PROTECT(R_MakeExternalPtr((void*) view, 
				    R_NilValue,
				    R_NilValue));
    UNPROTECT(1);
    return ans;
}


SEXP
setWidgetUrl(SEXP extp, SEXP url)
{
    qextp->setUrl(QString(CHAR(asChar(url))));
    return R_NilValue;
}



}



// In this version, we don't allow paint methods to call R code.  The
// painting details are determined at widget creation time.


#include <QtGui>
#include <QPainter>
#include <QToolTip>

#include "QtPainter.hpp"
#include "conversion.h"
#include "basicWidgets.h"
#include "paintUtils.hpp"

#ifndef WIN32
#include <unistd.h>
#endif

#include <R.h>
#include <Rinternals.h>

using namespace QViz;

// QGL::DoubleBuffer
// QGL::DepthBuffer
// QGL::Rgba
// QGL::AlphaChannel
// QGL::AccumBuffer
// QGL::StencilBuffer
// QGL::StereoBuffers
// QGL::DirectRendering
// QGL::HasOverlay
// QGL::SampleBuffers
// QGL::SingleBuffer
// QGL::NoDepthBuffer
// QGL::ColorIndex
// QGL::NoAlphaChannel
// QGL::NoAccumBuffer
// QGL::NoStencilBuffer
// QGL::NoStereoBuffers
// QGL::IndirectRendering
// QGL::NoOverlay
// QGL::NoSampleBuffers

// | QGL::AlphaChannel



// #ifdef QV_USE_OPENGL
//     : QGLWidget(QGLFormat(QGL::SingleBuffer | QGL::SampleBuffers), parent)
// #else 

QVBasicWidget::QVBasicWidget(SEXP rinfo,
			     double x1, double x2,
			     double y1, double y2,
			     int xexpand, int yexpand,
			     int minheight, int minwidth,
			     int margin,
			     QWidget *parent)
  : QWidget(parent)
{
  glayout = new QGridLayout;
  glayout->setHorizontalSpacing(0);
  glayout->setVerticalSpacing(0);
  glayout->setContentsMargins(margin, margin, margin, margin);
  setMouseTracking(true);
  setLayout(glayout);
  setRenderInfo(rinfo);
  setFocusPolicy(Qt::StrongFocus);
  setX1(x1);
  setX2(x2);
  setY1(y1);
  setY2(y2);
  setExpansion(xexpand, yexpand);
  setMinimumHeight(minheight);
  setMinimumWidth(minwidth);
  setReferenceFlag(1);
}

QVBasicWidget::~QVBasicWidget()
{
  // go through list of children; for those that have an R
  // reference, reparent them to be top-level widgets (so that they
  // are not deleted)

  QVBasicWidget *c;
  const QObjectList clist = children();
  // Rprintf("%d children(", clist.size());
  for (int i = 0; i < clist.size(); ++i) {
    c = qobject_cast<QVBasicWidget *>(clist[i]); // 0 if cast was unsuccessful
    if (c) {
      if (c->getReferenceFlag()) {
        // Rprintf("+");
        c->setParent(0);
      }
      // else Rprintf("*");
    }
    // else Rprintf("-");
  }
  // Rprintf(")");
  R_ReleaseObject(render_info);
  delete glayout;
}

void QVBasicWidget::setExpansion(int x, int y) 
{ 
  xexpand = x;
  yexpand = y;
  QSizePolicy::Policy 
    hpolicy = QSizePolicy::Preferred,
    vpolicy = QSizePolicy::Preferred;
  if (x) hpolicy = QSizePolicy::Expanding;
  if (y) vpolicy = QSizePolicy::Expanding;
  setSizePolicy(hpolicy, vpolicy);
}




void QVBasicWidget::paintEvent(QPaintEvent *)
{
  QCursor oldcursor = cursor();
  setCursor(Qt::BusyCursor);

  QString cname;
  SEXP compi, args, render;
  int i, n;
  PROTECT(render = getRenderInfo());
  n = length(render);
    
  if (n > 0) {
    QtPainter painter(this);
    QRectF bounds = QRectF(QPointF(getX1(), getY1()),
                           QPointF(getX2(), getY2()));
    painter.setLimits(bounds);
    // Rprintf("%d components.\n", n); // DEBUG    
    for (i = 0; i < n; i++) {
      compi = VECTOR_ELT(render, i);
      cname = QString(CHAR(asChar(VECTOR_ELT(compi, 0))));
      // Rprintf("%s\n", CHAR(asChar(VECTOR_ELT(compi, 0)))); // DEBUG
      args = VECTOR_ELT(compi, 1); // list
      // cname can be: panelPoints, panelLines, setPlotColor,
      // etc. The main point being that the choices are limited, and
      // args is interpreted accordingly.
      if (cname == QString("panelPoints")) {
        QPainterPath path;
        path.addEllipse(0, 0, 10, 10);
        painter.setGlyphSize(1.0);
        painter.drawGlyphs(path, REAL(VECTOR_ELT(args, 0)),
                           REAL(VECTOR_ELT(args, 1)), NULL,
                           NULL, NULL, length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("panelLines")) {
        PaintUtils::drawPolylines(&painter, REAL(VECTOR_ELT(args, 0)),
                                  REAL(VECTOR_ELT(args, 1)), NULL,
                                  length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("panelSegments")) {
        painter.drawSegments(REAL(VECTOR_ELT(args, 0)),
                             REAL(VECTOR_ELT(args, 1)),
                             REAL(VECTOR_ELT(args, 2)),
                             REAL(VECTOR_ELT(args, 3)),
                             length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("panelText")) {
        // could handle vadj and hadj here, perhaps if qvText()
        // were reimplemented in C
        painter.drawText(asStringArray(VECTOR_ELT(args, 2)),
                         REAL(VECTOR_ELT(args, 0)),
                         REAL(VECTOR_ELT(args, 1)),
                         length(VECTOR_ELT(args, 0)),
                         Qt::AlignCenter,
                         asReal(VECTOR_ELT(args, 3))); // rot
      }
      else if (cname == QString("panelPolyline")) {
        painter.drawPolyline(REAL(VECTOR_ELT(args, 0)),
                             REAL(VECTOR_ELT(args, 1)),
                             length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("panelPolygon")) {
        PaintUtils::drawPolygons(&painter, REAL(VECTOR_ELT(args, 0)),
                                 REAL(VECTOR_ELT(args, 1)),
                                 NULL, NULL,
                                 length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("panelRect")) {
        painter.drawRectangles(REAL(VECTOR_ELT(args, 0)),
                               REAL(VECTOR_ELT(args, 1)),
                               REAL(VECTOR_ELT(args, 2)),
                               REAL(VECTOR_ELT(args, 3)),
                               length(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setAntialias")) {
        painter.setAntialias(asInteger(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setX1")) {
        setX1(asReal(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setX2")) {
        setX2(asReal(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setY1")) {
        setY1(asReal(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setY2")) {
        setY2(asReal(VECTOR_ELT(args, 0)));
      }
      else if (cname == QString("setFont")) {
        painter.setFont(QFont(QString(CHAR(asChar(VECTOR_ELT(args, 0)))),
                              asInteger(VECTOR_ELT(args, 1)),
                              asInteger(VECTOR_ELT(args, 2)),
                              asInteger(VECTOR_ELT(args, 3)) == 1));
      }
      else if (cname == QString("setPlotColor")) { // RGBA
        painter.setStrokeColor(QColor(asInteger(VECTOR_ELT(args, 0)),
                                      asInteger(VECTOR_ELT(args, 1)),
                                      asInteger(VECTOR_ELT(args, 2)),
                                      asInteger(VECTOR_ELT(args, 3))));
      }
      else if (cname == QString("setFillColor")) {
        painter.setFillColor(QColor(asInteger(VECTOR_ELT(args, 0)),
                                    asInteger(VECTOR_ELT(args, 1)),
                                    asInteger(VECTOR_ELT(args, 2)),
                                    asInteger(VECTOR_ELT(args, 3))));
      }
    }
  }
  setCursor(oldcursor);
  if (hasFocus()) {
    QPainter qPainter(this);
    QBrush brush = qPainter.brush();
    brush.setColor(QColor(100, 100, 100, 10));
    qPainter.setBrush(brush);
    qPainter.drawRect(qPainter.window());
  }
  UNPROTECT(1);
  return;
}


void QVBasicWidget::mouseMoveEvent(QMouseEvent *e)
{
  e->accept();
  //     if (e->button() == Qt::LeftButton) {
  // 	// QToolTip::showText(e->pos(), QString("I am a tooltip"), this, rect());
  // 	QToolTip::showText(e->globalPos(), 
  // 			   QString("POS: ") + QString(e->x()) + QString(e->y()), 
  // 			   this, rect());
  //     }
}


void QVBasicWidget::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == Qt::LeftButton) {
    QToolTip::showText(e->globalPos(), 
                       QString("POS: ") + QString::number(e->x()) + QString::number(e->y()), 
                       this, rect());
    e->accept();
  }
}




#include "basicWidgets.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Error.h>

// // #define qextp ((QVBasicWidget *) R_ExternalPtrAddr(extp))
// #define asQWidget(x) (qobject_cast<QWidget *>((QObject*) R_ExternalPtrAddr(x))); 
// #define asQVBasicWidget(x) (qobject_cast<QVBasicWidget *>((QObject*) R_ExternalPtrAddr(x)));

static QWidget *asQWidget(SEXP x) {
  QWidget *ans = qobject_cast<QWidget *>((QObject*) R_ExternalPtrAddr(x));
  if (!ans) warning("Coercion to QWidget failed");
  return ans;
}

static QVBasicWidget *asQVBasicWidget(SEXP x) {
  QVBasicWidget *ans = qobject_cast<QVBasicWidget *>((QObject*) R_ExternalPtrAddr(x));
  if (!ans) warning("Coercion to QVBasicWidget failed");
  return ans;
}



// R-level interface to constructor


extern "C" {


  SEXP
  newBasicWidget(SEXP rinfo,
                 SEXP x1, SEXP x2,
                 SEXP y1, SEXP y2,
                 SEXP xexpand, SEXP yexpand,
                 SEXP minheight, SEXP minwidth, 
                 SEXP margin,
                 SEXP tag)
  {
    SEXP 
      ans, 
      drinfo = duplicate(rinfo),
      dtag = PROTECT(duplicate(tag));
    R_PreserveObject(drinfo);
    QVBasicWidget 
      *qvbw = new QVBasicWidget(drinfo,
                                asReal(x1),
                                asReal(x2),
                                asReal(y1),
                                asReal(y2),
                                asInteger(xexpand),
                                asInteger(yexpand),
                                asInteger(minheight),
                                asInteger(minwidth),
                                asInteger(margin),
                                0);
    ans = PROTECT(R_MakeExternalPtr((void*) qvbw, 
				    dtag,
				    R_NilValue));
    UNPROTECT(2);
    return ans;
  }

  SEXP
  setWidgetMinHeight(SEXP extp, SEXP x)
  {
    QWidget *w = asQWidget(extp);
    w->setMinimumHeight(asInteger(x));
    return R_NilValue;
  }

  SEXP
  setWidgetMinWidth(SEXP extp, SEXP x)
  {
    QWidget *w = asQWidget(extp);
    w->setMinimumWidth(asInteger(x));
    return R_NilValue;
  }

  SEXP
  setWidgetSize(SEXP extp, SEXP w, SEXP h)
  {
    QWidget *e = asQWidget(extp);
    int ww, hh;
    if (w == R_NilValue) ww = e->width();
    else ww = asInteger(w);
    if (h == R_NilValue) hh = e->height();
    else hh = asInteger(h);
    e->resize(ww, hh);
    return R_NilValue;
  }

  SEXP
  finalizeBasicWidget(SEXP extp)
  {
    // (1) Set flag in widget saying that R is no longer pointing to
    // it.  (2) Delete the widget it if it is a top-level widget, but
    // not otherwise.

    // Widgets can be deleted here, but could also deleted by Qt
    // (typically when deleting its parent).  The idea is that Qt
    // should never delete a widget if it has an R reference pointing
    // to it (a delete event should eventually be called on it when
    // the finalizer runs).  On the other hand, child widgets will not
    // be deleted here, but will be deleted by Qt when their parent is
    // finalized and deleted

    QVBasicWidget *p = asQVBasicWidget(extp);
    // Rprintf("Coercion OK. ");
    if (!p) return R_NilValue;
    // Rprintf("Non-null. ");
    SEXP oldtag = PROTECT(R_ExternalPtrTag(extp));
    R_SetExternalPtrTag(extp, R_NilValue);
    p->setReferenceFlag(0);
    // Rprintf("Unset ref. ");
    R_ClearExternalPtr(extp);
    if (!p->parent()) { // top-level widget, so delete
      // Rprintf("Top-level; Deleting. ");
      delete p;
    }
    UNPROTECT(1);
    // Rprintf("Done.\n");
    return oldtag;
  }


  SEXP
  getWidgetRenderInfo(SEXP extp)
  {
    QVBasicWidget *w = asQVBasicWidget(extp);
    if (w) return w->getRenderInfo();
    else return R_NilValue;
  }

  SEXP
  addWidgetInLayout(SEXP w, SEXP target,
                    SEXP row, 
                    SEXP col, 
                    SEXP nrow, 
                    SEXP ncol)
  {
    QVBasicWidget *wt = asQVBasicWidget(target);
    QWidget *ww = asQWidget(w);
    if (wt && ww) {
      wt->getLayout()->addWidget(ww,
                                 asInteger(row),
                                 asInteger(col),
                                 asInteger(nrow),
                                 asInteger(ncol));
    }
    return R_NilValue;    
  }



}



#include <QPen>
#include <QBrush>
#include <QFont>
#include "paintPrimitives.h"

// pixel to native transformation, x and y
#define pnx(x) w * (x - x1) / dx
#define pny(y) h * (y2 - y) / dy


void QViz::setPlotColor(QPainter &painter, QColor c)
{
  QPen pen = painter.pen();
  pen.setColor(c);
  painter.setPen(pen);
}

void QViz::setFillColor(QPainter &painter, QColor c)
{
  QBrush brush = painter.brush();
  if (c.alpha() == 0) 
    brush.setStyle(Qt::NoBrush);
  else {
    brush.setStyle(Qt::SolidPattern); // FIXME: more options?
    brush.setColor(c);
  }
  painter.setBrush(brush);
}

void QViz::setFont(QPainter &painter, QFont font)
{
  painter.setFont(font);
}




void 
QViz::panelPoints(QPainter &painter, 
		  double x1, double x2, double y1, double y2,
		  double *px, double *py, int n)
{
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    rad = 5.0, // * cex (FIXME: how?)
    dx = x2 - x1,
    dy = y2 - y1;
  // Rprintf("%d points\n", n);
  // NB: won't use setViewport() because can't control aspect
  for (i = 0; i < n; i++) {
    painter.drawEllipse(QRectF(pnx(px[i]) - rad, 
                               pny(py[i]) - rad, 
                               2 * rad, 2 * rad));
  }
}

// panelPolyline is a better alternative (for non-NA)
void 
QViz::panelLines(QPainter &painter,
		 double x1, double x2, double y1, double y2,
		 double *px, double *py, int n)
{
  if (n < 2) return;
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    dx = x2 - x1,
    dy = y2 - y1;
  for (i = 1; i < n; i++) {
    painter.drawLine(QLineF(pnx(px[i-1]),
                            pny(py[i-1]),
                            pnx(px[i]),
                            pny(py[i])));
  }
}


void 
QViz::panelSegments(QPainter &painter,
		    double x1, double x2, double y1, double y2,
		    double *px0, double *py0, 
		    double *px1, double *py1, 
		    int n)
{
  if (n < 1) return;
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    dx = x2 - x1,
    dy = y2 - y1;
  for (i = 0; i < n; i++) {
    painter.drawLine(QLineF(pnx(px0[i]), pny(py0[i]),
                            pnx(px1[i]), pny(py1[i])));
  }
}



void 
QViz::panelText(QPainter &painter,
		double x1, double x2, double y1, double y2,
		double *px, double *py, int n, SEXP strs,
		double rot, double hadj, double vadj)
{
  QString qstr;
  QRectF brect;
  double x, y;
  if (n < 1) return;
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    dx = x2 - x1,
    dy = y2 - y1;
  for (i = 0; i < n; i++) {
    x = w * (px[i] - x1) / dx; 
    y = h * (y2 - py[i]) / dy; 
    qstr = QString::fromLocal8Bit(CHAR(asChar(STRING_ELT(strs, i))));
    painter.save();
    painter.translate(x, y);
    painter.rotate(-rot);
    brect = painter.boundingRect(0, 0, 1, 1,
                                 Qt::AlignLeft | Qt::AlignBottom, qstr);
    painter.drawText(QPointF(-hadj * brect.width(), 
                             vadj * brect.height()), 
                     qstr);
    painter.restore();
  }
}


void 
QViz::panelPolygon(QPainter &painter,
		   double x1, double x2, double y1, double y2,
		   double *px, double *py, int n)
{
  if (n < 2) return;
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    dx = x2 - x1,
    dy = y2 - y1;
  QPolygonF points;
  for (i = 0; i < n; i++)
    points << QPointF(pnx(px[i]), pny(py[i]));
  painter.drawPolygon(points);
}

void 
QViz::panelPolyline(QPainter &painter,
		    double x1, double x2, double y1, double y2,
		    double *px, double *py, int n)
{
  if (n < 2) return;
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    dx = x2 - x1,
    dy = y2 - y1;
  QPolygonF points;
  for (i = 0; i < n; i++)
    points << QPointF(pnx(px[i]), pny(py[i]));
  painter.drawPolyline(points);
}

void 
QViz::panelRect(QPainter &painter,
		double x1, double x2, double y1, double y2,
		double *px, double *py, // centers 
		double *pw, double *ph, // widths/heights
		int n)
{
  int i, 
    w = painter.window().width(),
    h = painter.window().height();
  double 
    rad = 10.0,
    dx = x2 - x1,
    dy = y2 - y1;
  // FIXME: maybe use drawRects()? How?
  for (i = 0; i < n; i++) {
    // drawRect(QRectF(x, y, w, h)); (x,y) = top-left
    double 
      xleft   = pnx(px[i] - 0.5 * pw[i]),
      xright  = pnx(px[i] + 0.5 * pw[i]),
      ybottom = pny(py[i] - 0.5 * ph[i]),
      ytop    = pny(py[i] + 0.5 * ph[i]);
    painter.drawRect(QRectF(xleft, ytop, 
                            xright - xleft,
                            ybottom - ytop));
  }
}




#include <QObject>
#include <QWidget>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QSvgGenerator>

#include <QSize>
#include <QFont>
#include <QFontMetrics>

#ifndef WIN32
#include <unistd.h>
#endif

#include "wrappers.h"

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Error.h>

static
QString sexp2qstring(SEXP s) {
  return QString::fromLocal8Bit(CHAR(asChar(s)));
}

// #define qextp ((QWidget *) R_ExternalPtrAddr(extp))


extern "C" {




  // font stuff - may belong in a separate file


  SEXP
  stringBoundingBox(SEXP s, SEXP font, SEXP width, SEXP height)
  {
    int i;
    QFont f = QFont(QString(CHAR(asChar(VECTOR_ELT(font, 0)))), // family
		    asInteger(VECTOR_ELT(font, 1)),             // pointsize
		    asInteger(VECTOR_ELT(font, 2)),             // weight
		    asInteger(VECTOR_ELT(font, 3)) == 1);      // italic? true/false
    QFontMetrics fm(f);
    for (i = 0; i < length(s); i++) {
      QSize size = fm.size(0, sexp2qstring(STRING_ELT(s, i)));
      INTEGER(width)[i] = size.width();
      INTEGER(height)[i] = size.height();
    }
    return R_NilValue;
  }



}


// interface using the Graphics View framework.  For now, this is just
// an attempt at getting cheap zoom/pan type operations.  

// Generally speaking, this seems to be the right model/view approach
// to doing dynamic graphics: from what I can tell, the principle
// would be to define a "scene" (analogous to a dataset) and muliple
// "views" on it.  The R-level API might be to create an external
// pointer for the data (scene), and views that would link to it.


#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGLWidget>
#include <QGraphicsWidget>
#include <QGraphicsTextItem>

#include "Layer.hpp"
#include "PlotView.hpp"
#include "wrappers.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include <R.h>
#include <Rinternals.h>

// #define qextp ((QWidget *) R_ExternalPtrAddr(extp))

// R-level interface to constructor


extern "C" {

  SEXP
  scene_addPoints(SEXP scene, SEXP x, SEXP y, SEXP radius)
  {
    QGraphicsScene* s = 
      qobject_cast<QGraphicsScene *>((QObject*) R_ExternalPtrAddr(scene));
    if (s) {
      int i, n = length(x);
      for (i = 0; i < n; i++) {
        // QGraphicsEllipseItem *item = s->addEllipse(REAL(x)[i], REAL(y)[i], REAL(radius)[0], REAL(radius)[0]);
        QGraphicsEllipseItem *item = s->addEllipse(0.0, 0.0, REAL(radius)[0], REAL(radius)[0]);
        item->setPos(REAL(x)[i], REAL(y)[i]);
        item->setFlags(QGraphicsItem::ItemIsSelectable | 
                       QGraphicsItem::ItemIgnoresTransformations);
      }
    }
    return R_NilValue;
  }


  SEXP
  scene_addLines(SEXP scene, SEXP x, SEXP y, SEXP lwd)
  {
    int nlwd = length(lwd);
    QGraphicsScene* s = 
      qobject_cast<QGraphicsScene *>((QObject*) R_ExternalPtrAddr(scene));
    if (s) {
      int i, n = length(x);
      for (i = 1; i < n; i++) {
        QGraphicsLineItem *item = s->addLine(REAL(x)[i-1], REAL(y)[i-1], 
                                             REAL(x)[i],   REAL(y)[i], 
                                             QPen(Qt::black, REAL(lwd)[(i-1) % nlwd]));
        // 	    item->setFlags(QGraphicsItem::ItemIsSelectable | 
        // 			   QGraphicsItem::ItemIgnoresTransformations);
      }
    }
    return R_NilValue;
  }


  SEXP
  scene_addSegments(SEXP scene, SEXP x1, SEXP y1, SEXP x2, SEXP y2, SEXP lwd)
  {
    int nlwd = length(lwd);
    QGraphicsScene* s = 
      qobject_cast<QGraphicsScene *>((QObject*) R_ExternalPtrAddr(scene));
    if (s) {
      int i, n = length(x1);
      for (i = 0; i < n; i++) {
        QGraphicsLineItem *item = s->addLine(REAL(x1)[i], REAL(y1)[i], REAL(x2)[i], REAL(y2)[i], 
                                             QPen(Qt::black, REAL(lwd)[(i-1) % nlwd]));
        // 	    item->setFlags(QGraphicsItem::ItemIsSelectable | 
        // 			   QGraphicsItem::ItemIgnoresTransformations);
      }
    }
    return R_NilValue;
  }



  // QGraphicsPolygonItem * addPolygon ( const QPolygonF & polygon, const QPen & pen = QPen(), const QBrush & brush = QBrush() )


  SEXP
  scene_addRect(SEXP scene, SEXP x, SEXP y, SEXP w, SEXP h)
  {
    int nw = length(w), nh = length(h);
    QGraphicsScene* s = 
      qobject_cast<QGraphicsScene *>((QObject*) R_ExternalPtrAddr(scene));
    if (s) {
      int i, n = length(x);
      for (i = 0; i < n; i++) {
        QGraphicsRectItem *item = s->addRect(REAL(x)[i], REAL(y)[i], 
                                             REAL(w)[i % nw], REAL(h)[i % nh]);
        // 	    item->setFlags(QGraphicsItem::ItemIsSelectable | 
        // 			   QGraphicsItem::ItemIgnoresTransformations);
      }
    }
    return R_NilValue;
  }

  SEXP
  scene_addText(SEXP scene, SEXP x, SEXP y, SEXP labels, SEXP html)
  {
    int nlab = length(labels);
    QGraphicsScene* s = 
      qobject_cast<QGraphicsScene *>((QObject*) R_ExternalPtrAddr(scene));
    if (s) {
      int i, n = length(x);
      for (i = 0; i < n; i++) {
        QGraphicsTextItem *ti = s->addText(QString());
        ti->setFont(QFont("Arial"));
        if (LOGICAL(html)[0]) {
          ti->setHtml(QString::fromLocal8Bit(CHAR(asChar(STRING_ELT(labels, i % nlab)))));
          ti->setOpenExternalLinks(true);
          ti->setToolTip("I am HTML!");
        }
        else {
          ti->setPlainText(QString::fromLocal8Bit(CHAR(asChar(STRING_ELT(labels, i % nlab)))));
        }
        ti->setPos(REAL(x)[i], REAL(y)[i]);
        ti->setFlags(QGraphicsItem::ItemIsMovable | 
                     QGraphicsItem::ItemIsSelectable | 
                     QGraphicsItem::ItemIsFocusable | 
                     QGraphicsItem::ItemIgnoresTransformations);
      }
    }
    return R_NilValue;
  }




  SEXP
  setViewTransform(SEXP extp,
                   SEXP xscale,
                   SEXP yscale,
                   SEXP rotate,
                   SEXP translate)
  {
    QGraphicsView *view = 
      qobject_cast<QGraphicsView *>((QObject*) R_ExternalPtrAddr(extp));
    // shear ( qreal sh, qreal sv )
    view->scale(REAL(xscale)[0], REAL(yscale)[0]);
    view->rotate(REAL(rotate)[0]);
    view->translate(REAL(translate)[0], REAL(translate)[1]);
    return R_NilValue;
  }

  SEXP
  view_setDragMode(SEXP extp, SEXP mode)
  {
    QGraphicsView *view = 
      qobject_cast<QGraphicsView *>((QObject*) R_ExternalPtrAddr(extp));
    if (view) {
      int m = INTEGER(mode)[0];
      if (m == 0)
        view->setDragMode(QGraphicsView::NoDrag);
      else if (m == 1)
        view->setDragMode(QGraphicsView::ScrollHandDrag);
      else if (m == 2)
        view->setDragMode(QGraphicsView::RubberBandDrag);
    }
    return R_NilValue;
  }

  SEXP
  view_setAntialias(SEXP extp, SEXP mode)
  {
    QGraphicsView *view = 
      qobject_cast<QGraphicsView *>((QObject*) R_ExternalPtrAddr(extp));
    if (view) {
      if (LOGICAL(mode)[0]) {
        view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing); 
      }
      else {
        view->setRenderHints(QPainter::TextAntialiasing); 
      }
    }
    return R_NilValue;
  }
