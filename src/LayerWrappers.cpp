#include "RLayer.hpp"
#include "PlotView.hpp"
#include "conversion.h"

#include <QGraphicsView>

using namespace QViz;

extern "C" {

  /* How to manage QGraphics* memory? */
  /* QGraphicsView: just like any other widget.
     QGraphicsScene: should be "owned" by the views. But multiple
     ownership is not provided by Qt. Could implement a reference
     counting scheme. Alternative: place in protected slot of external
     pointer, but would have to unprotect when externalptr goes away,
     even if view widget does not (still has a parent).
     QGraphicsItem: owned by parent item, if there is one, otherwise
     by the scene.
  */
  
  SEXP qt_qlayer(SEXP paint, SEXP keyPress, SEXP keyRelease,
                   SEXP mouseDoubleClick, SEXP mouseMove, SEXP mousePress,
                   SEXP mouseRelease, SEXP wheel)
  {
    RLayer *layer = new RLayer(paint, keyPress, keyRelease, mouseDoubleClick,
              mouseMove, mousePress, mouseRelease, wheel);
    return wrapQGraphicsWidget(layer);
  }
  
  SEXP qt_qupdate_QGraphicsItem(SEXP rself) {
    QGraphicsWidget *item = unwrapQObject(rself, QGraphicsWidget);
    // HACK: purge the cache before updating. QGraphicsScene does not
    // seem to update the cache properly when there are multiple
    // views. This is not very efficient, but usually one is not
    // caching layers that are frequently updated.
    QGraphicsItem::CacheMode mode = item->cacheMode();
    item->setCacheMode(QGraphicsItem::NoCache);
    item->setCacheMode(mode);
    item->update();
    return rself;
  }
  
  // FIXME: should be function of the scene
  SEXP qt_qpaintingView_QGraphicsItem(SEXP rself) {
    QGraphicsWidget *self = unwrapQObject(rself, QGraphicsWidget);
    QList<QGraphicsView *> views = self->scene()->views();
    PlotView *paintingView = NULL;
    for (int i = 0; i < views.size() && !paintingView; i++) {
      PlotView *view = qobject_cast<PlotView *>(views[i]);
      if (view && view->isPainting())
        paintingView = view;
    }
    return paintingView ? wrapQWidget(paintingView) : R_NilValue;
  }

  // all the way to the viewport, useful for moving between pixels and
  // data in event callbacks
  SEXP qt_qdeviceMatrix_QGraphicsItem(SEXP rself, SEXP rview, SEXP rinverted) {
    QGraphicsWidget *self = unwrapQObject(rself, QGraphicsWidget);
    QGraphicsView *view = unwrapQObject(rview, QGraphicsView);
    // This method is buggy (assumes 'self' ignores transformations)
    //QMatrix mat = self->deviceTransform(view->viewportTransform()).toAffine();
    QMatrix mat = (self->sceneTransform() *
                   view->viewportTransform()).toAffine();
    return asRMatrix(mat, asLogical(rinverted));
  }

  // just data to parent (layout/scene) coordinates, for size calculations
  SEXP qt_qmatrix_QGraphicsItem(SEXP rself, SEXP rinverted) {
    QGraphicsWidget *self = unwrapQObject(rself, QGraphicsWidget);
    return asRMatrix(self->transform().toAffine(), asLogical(rinverted));
  }
  
  SEXP qt_qsetGeometry_QGraphicsWidget(SEXP rself, SEXP rx)
  {
    QGraphicsWidget *widget = unwrapQObject(rself, QGraphicsWidget);
    widget->setGeometry(QRectF(QPointF(REAL(rx)[0], REAL(rx)[2]),
                               QPointF(REAL(rx)[1], REAL(rx)[3])));
    return rself;
  }

  SEXP qt_qgeometry_QGraphicsWidget(SEXP rself) {
    QGraphicsWidget *widget = unwrapQObject(rself, QGraphicsWidget);
    return asRRect(widget->geometry());
  }

  SEXP qt_qboundingRect_QGraphicsItem(SEXP rself) {
    QGraphicsWidget *widget = unwrapQObject(rself, QGraphicsWidget);
    return asRRect(widget->boundingRect());
  }
  
  SEXP qt_qsetLimits_Layer(SEXP rself, SEXP xlim, SEXP ylim) {
    Layer *layer = unwrapQObject(rself, Layer);
    layer->setLimits(REAL(xlim)[0], REAL(ylim)[0], REAL(xlim)[1],
                     REAL(ylim)[1]);
    return rself;
  }

  SEXP qt_qlimits_Layer(SEXP rself) {
    Layer *layer = unwrapQObject(rself, Layer);
    return asRRect(layer->limits());
  }

  SEXP qt_qcolStretch_Layer(SEXP rself) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    SEXP ans = allocVector(INTSXP, layout->columnCount());
    for (int i = 0; i < length(ans); i++) {
      INTEGER(ans)[i] = layout->columnStretchFactor(i);
    }
    return ans;
  }
  SEXP qt_qrowStretch_Layer(SEXP rself) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    SEXP ans = allocVector(INTSXP, layout->rowCount());
    for (int i = 0; i < length(ans); i++) {
      INTEGER(ans)[i] = layout->rowStretchFactor(i);
    }
    return ans;
  }
  
  SEXP qt_qsetColStretch_Layer(SEXP rself, SEXP rstretch) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    for (int i = 0; i < length(rstretch); i++) {
      layout->setColumnStretchFactor(i, INTEGER(rstretch)[i]);
    }
    return rself;
  }
  SEXP qt_qsetRowStretch_Layer(SEXP rself, SEXP rstretch) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    for (int i = 0; i < length(rstretch); i++) {
      layout->setRowStretchFactor(i, INTEGER(rstretch)[i]);
    }
    return rself;
  }

  SEXP qt_qsetHorizontalSpacing_Layer(SEXP rself, SEXP rspacing) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    layout->setHorizontalSpacing(asReal(rspacing));
    return rself;
  }
  SEXP qt_qsetVerticalSpacing_Layer(SEXP rself, SEXP rspacing) {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    layout->setVerticalSpacing(asReal(rspacing));
    return rself;
  }
  
  SEXP qt_qaddChildItem_Layer(SEXP rself, SEXP ritem, SEXP rrow, SEXP rcol,
                              SEXP rrowSpan, SEXP rcolSpan)
  {
    Layer *layer = unwrapQObject(rself, Layer);
    QGraphicsWidget *item = unwrapQObject(ritem, QGraphicsWidget);
    // this cast is a bit dangerous.. how is this handled in general?
    // could we maintain our own layout?
    QGraphicsGridLayout *layout = (QGraphicsGridLayout *)layer->layout();
    layout->addItem(item, asInteger(rrow), asInteger(rcol), asInteger(rrowSpan),
                    asInteger(rcolSpan));
    //layout->addItem(new QGraphicsWidget, 1, 1);
    return rself;
  }

  SEXP qt_qitemsAtPoint_Layer(SEXP rself, SEXP rx, SEXP ry) {
    Layer *self = unwrapQObject(rself, Layer);
    SEXP ans;
    QVector<int> items = self->items(QPointF(asReal(rx), asReal(ry)));
    ans = allocVector(INTSXP, items.size());
    for (int i = 0; i < length(ans); i++)
      INTEGER(ans)[i] = items[i] + 1;
    return ans;
  }

  SEXP qt_qitemsInRect_Layer(SEXP rself, SEXP rx) {
    Layer *self = unwrapQObject(rself, Layer);
    SEXP ans;
    QVector<int> items = self->items(QRectF(QPointF(REAL(rx)[0], REAL(rx)[2]),
                   QPointF(REAL(rx)[1], REAL(rx)[3])));
    ans = allocVector(INTSXP, items.size());
    for (int i = 0; i < length(ans); i++) {
      INTEGER(ans)[i] = items[i] + 1;
    }
    return ans;
  }

  SEXP qt_qsetCacheMode_QGraphicsItem(SEXP rself, SEXP rmode) {
    QGraphicsWidget *item = unwrapQObject(rself, QGraphicsWidget);
    item->setCacheMode((QGraphicsItem::CacheMode)asInteger(rmode));
    return rself;
  }
  SEXP qt_qcacheMode_QGraphicsItem(SEXP rself) {
    QGraphicsWidget *item = unwrapQObject(rself, QGraphicsWidget);
    return ScalarInteger(item->cacheMode());
  }

  SEXP qt_qsetFocus_QGraphicsItem(SEXP rself) {
    QGraphicsWidget *item = unwrapQObject(rself, QGraphicsWidget);
    item->setFocus();
    return rself;
  }
}
