#include "RLayer.hpp"
#include "PlotView.hpp"

#include <QGraphicsGridLayout>
#include <QGraphicsView>

#include <qtbase.h>

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
  // REMOVE when able to get scene and viewport tform, multiply and invert
  SEXP qt_qdeviceMatrix_QGraphicsItem(SEXP rself, SEXP rview, SEXP rinverted) {
    QGraphicsItem *self = unwrapQGraphicsItem(rself, QGraphicsItem);
    QGraphicsView *view = unwrapQObject(rview, QGraphicsView);
    // This method is buggy (assumes 'self' ignores transformations)
    //QMatrix mat = self->deviceTransform(view->viewportTransform()).toAffine();
    QTransform mat = self->sceneTransform() * view->viewportTransform();
    return asRMatrix(mat.toAffine(), asLogical(rinverted));
  }
  
  SEXP qt_qsetLimits_Layer(SEXP rself, SEXP rrect) {
    Layer *layer = unwrapQObject(rself, Layer);
    layer->setLimits(asQRectF(rrect));
    return rself;
  }

  SEXP qt_qlimits_Layer(SEXP rself) {
    Layer *layer = unwrapQObject(rself, Layer);
    return asRRectF(layer->limits());
  }

  SEXP qt_qprimitivesAtPoint_Layer(SEXP rself, SEXP rpoint) {
    Layer *self = unwrapQObject(rself, Layer);
    SEXP ans;
    QVector<int> primitives = self->primitives(asQPointF(rpoint));
    ans = allocVector(INTSXP, primitives.size());
    for (int i = 0; i < length(ans); i++)
      INTEGER(ans)[i] = primitives[i] + 1;
    return ans;
  }

  SEXP qt_qprimitivesInRect_Layer(SEXP rself, SEXP rrect) {
    Layer *self = unwrapQObject(rself, Layer);
    SEXP ans;
    QVector<int> primitives = self->primitives(asQRectF(rrect));
    ans = allocVector(INTSXP, primitives.size());
    for (int i = 0; i < length(ans); i++) {
      INTEGER(ans)[i] = primitives[i] + 1;
    }
    return ans;
  }
  
  // REMOVE these functions after we can work with QGraphicsGridLayout
  // in qtgui.
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
  
  SEXP qt_qaddItem_Layer(SEXP rself, SEXP ritem, SEXP rrow, SEXP rcol,
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
}
