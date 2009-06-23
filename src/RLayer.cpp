#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include "RLayer.hpp"
#include "PlotView.hpp"
#include "conversion.h"

using namespace QViz;

#define PRESERVE_CALLBACK(x) \
  if (x ## _R != R_NilValue) R_PreserveObject(x ## _R)
#define RELEASE_CALLBACK(x) if (x ## _R != R_NilValue) R_ReleaseObject(x ## _R)

// at some point might want to do something special
#define wrapQGraphicsScene wrapQObject

RLayer::RLayer(SEXP paintEvent, SEXP keyPressEvent, SEXP keyReleaseEvent,
               SEXP mouseDoubleClickEvent, SEXP mouseMoveEvent,
               SEXP mousePressEvent, SEXP mouseReleaseEvent, SEXP wheelEvent)
  : paintEvent_R(paintEvent), keyPressEvent_R(keyPressEvent),
    keyReleaseEvent_R(keyReleaseEvent),
    mouseDoubleClickEvent_R(mouseDoubleClickEvent),
    mouseMoveEvent_R(mouseMoveEvent), mousePressEvent_R(mousePressEvent),
    mouseReleaseEvent_R(mouseReleaseEvent), wheelEvent_R(wheelEvent)
{
  if (keyPressEvent != R_NilValue || keyReleaseEvent != R_NilValue)
    setFocusPolicy(Qt::StrongFocus);
  if (mouseMoveEvent != R_NilValue)
    setAcceptHoverEvents(true);
  /* preserve every callback */
  PRESERVE_CALLBACK(paintEvent);
  PRESERVE_CALLBACK(keyPressEvent);
  PRESERVE_CALLBACK(keyReleaseEvent);
  PRESERVE_CALLBACK(mouseDoubleClickEvent);
  PRESERVE_CALLBACK(mouseMoveEvent);
  PRESERVE_CALLBACK(mousePressEvent);
  PRESERVE_CALLBACK(mouseReleaseEvent);
  PRESERVE_CALLBACK(wheelEvent);
}

RLayer::~RLayer() {
  /* release every callback */
  RELEASE_CALLBACK(paintEvent);
  RELEASE_CALLBACK(keyPressEvent);
  RELEASE_CALLBACK(keyReleaseEvent);
  RELEASE_CALLBACK(mouseDoubleClickEvent);
  RELEASE_CALLBACK(mouseMoveEvent);
  RELEASE_CALLBACK(mousePressEvent);
  RELEASE_CALLBACK(mouseReleaseEvent);
  RELEASE_CALLBACK(wheelEvent);
}

static SEXP viewForEvent(QGraphicsSceneEvent *event) {
  SEXP ans = R_NilValue;
  QGraphicsView *view = qobject_cast<QGraphicsView*>(event->widget()->parent());
  if (view)
    ans = wrapQWidget(view);
  return ans;
}

static const char* keyToString(int key); // moved to end, too long
  
static SEXP asRModifiers(Qt::KeyboardModifiers modifiers) {
  static const char * modNames[] = { "shift", "control", "alt", "meta",
                                     "keypad", "groupswitch", NULL };
  SEXP rmodifiers;
  PROTECT(rmodifiers = allocVector(LGLSXP, 6));
  LOGICAL(rmodifiers)[0] = modifiers & Qt::ShiftModifier;
  LOGICAL(rmodifiers)[1] = modifiers & Qt::ControlModifier;
  LOGICAL(rmodifiers)[2] = modifiers & Qt::AltModifier;
  LOGICAL(rmodifiers)[3] = modifiers & Qt::MetaModifier;
  LOGICAL(rmodifiers)[4] = modifiers & Qt::KeypadModifier;
  LOGICAL(rmodifiers)[5] = modifiers & Qt::GroupSwitchModifier;
  setAttrib(rmodifiers, R_NamesSymbol, asRStringArray(modNames));
  return rmodifiers;
}

// the 'screenPos' is global, we want viewport-relative
#define viewPosForEvent(event) \
  asRPoint(event->widget()->mapFromGlobal(event->screenPos()))


void invalidatePtr(SEXP ptr) {
  setAttrib(ptr, R_ClassSymbol, mkString("<invalid>"));
}

void RLayer::paintPlot(Painter *painter, QRectF exposed)
{
  SEXP e, etmp, rpainter;

  if (paintEvent_R == R_NilValue)
    return; // not painting (just for layout)

  PROTECT(e = allocVector(LANGSXP, 4));
  SETCAR(e, paintEvent_R);
  etmp = CDR(e);
  SETCAR(etmp, wrapQGraphicsWidget(this));
  etmp = CDR(etmp);
  rpainter = wrapPointer(painter, "Painter", NULL);
  SETCAR(etmp, rpainter);
  etmp = CDR(etmp);
  SETCAR(etmp, asRRect(exposed));

  R_tryEval(e, R_GlobalEnv, NULL);

  /* We cannot guarantee that these pointers will be valid */
  invalidatePtr(rpainter);

  UNPROTECT(1);
}

/* TODO: Wrap all event callbacks */

/*
void RLayer::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event )

void RLayer::dragEnterEvent ( QGraphicsSceneDragDropEvent * event )

void RLayer::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )

void RLayer::dragMoveEvent ( QGraphicsSceneDragDropEvent * event )

void RLayer::dropEvent ( QGraphicsSceneDragDropEvent * event )

void RLayer::focusInEvent ( QFocusEvent * event )

void RLayer::focusOutEvent ( QFocusEvent * event )

void RLayer::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )

void RLayer::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )

void RLayer::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
*/

/* Just implement these for now ... */

void RLayer::dispatchKeyEvent(SEXP closure, QKeyEvent *event) {
  static const char * eventNames[] = { "item", "key", "modifiers", NULL };
  SEXP e, etmp, revent;
  
  if (closure == R_NilValue)
    return;

  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, closure);
  etmp = CDR(e);

  revent = allocVector(VECSXP, 3);
  SETCAR(etmp, revent);

  SET_VECTOR_ELT(revent, 0, wrapQGraphicsWidget(this));
  SET_VECTOR_ELT(revent, 1, mkString(keyToString(event->key())));
  SET_VECTOR_ELT(revent, 2, asRModifiers(event->modifiers()));
  
  setAttrib(revent, R_NamesSymbol, asRStringArray(eventNames));
  
  R_tryEval(e, R_GlobalEnv, NULL);
  
  UNPROTECT(1);
}

void RLayer::keyPressEvent ( QKeyEvent * event ) {
  dispatchKeyEvent(keyPressEvent_R, event);
}

void RLayer::keyReleaseEvent ( QKeyEvent * event ) {
  dispatchKeyEvent(keyReleaseEvent_R, event);
}


void RLayer::dispatchMouseEvent(SEXP closure, QGraphicsSceneMouseEvent *event) {
  static const char * eventNames[]
    = { "item", "pos", "screenPos", "buttons", "modifiers", "view", NULL };
  SEXP e, etmp, revent;

  if (closure == R_NilValue) {
    return;
  }
  
  /* interesting parameters not passed:
     last mouse position
     position when certain button pressed
  */
  
  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, closure);
  etmp = CDR(e);
  revent = allocVector(VECSXP, 6);
  SETCAR(etmp, revent);
  
  SET_VECTOR_ELT(revent, 0, wrapQGraphicsWidget(this));
  SET_VECTOR_ELT(revent, 1, asRPoint(event->pos()));
  SET_VECTOR_ELT(revent, 2, viewPosForEvent(event));
  SET_VECTOR_ELT(revent, 3, ScalarInteger(event->buttons()));
  SET_VECTOR_ELT(revent, 4, asRModifiers(event->modifiers()));
  SET_VECTOR_ELT(revent, 5, viewForEvent(event));
  
  setAttrib(revent, R_NamesSymbol, asRStringArray(eventNames));
  
  R_tryEval(e, R_GlobalEnv, NULL);
  
  UNPROTECT(1);
}

void RLayer::dispatchHoverEvent(SEXP closure, QGraphicsSceneHoverEvent *event) {
  static const char * eventNames[]
    = { "item", "pos", "screenPos", "buttons", "modifiers", "view", NULL };
  SEXP e, etmp, revent;

  if (closure == R_NilValue) {
    return;
  }
  
  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, closure);
  etmp = CDR(e);
  revent = allocVector(VECSXP, 6);
  SETCAR(etmp, revent);
  
  SET_VECTOR_ELT(revent, 0, wrapQGraphicsWidget(this));
  SET_VECTOR_ELT(revent, 1, asRPoint(event->pos()));
  SET_VECTOR_ELT(revent, 2, viewPosForEvent(event));
  SET_VECTOR_ELT(revent, 4, asRModifiers(event->modifiers()));
  SET_VECTOR_ELT(revent, 5, viewForEvent(event));
  
  setAttrib(revent, R_NamesSymbol, asRStringArray(eventNames));
  
  R_tryEval(e, R_GlobalEnv, NULL);
  
  UNPROTECT(1);
}

void RLayer::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseDoubleClickEvent_R == R_NilValue)
    QGraphicsItem::mouseDoubleClickEvent(event);
  else dispatchMouseEvent(mouseDoubleClickEvent_R, event);
}

void RLayer::hoverMoveEvent ( QGraphicsSceneHoverEvent * event ) {
  if (mouseMoveEvent_R == R_NilValue)  
    QGraphicsItem::hoverMoveEvent(event);
  else dispatchHoverEvent(mouseMoveEvent_R, event);
}

/* NOTE: this is really more like a 'dragging' event */
void RLayer::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseMoveEvent_R == R_NilValue)  
    QGraphicsItem::mouseMoveEvent(event);
  else dispatchMouseEvent(mouseMoveEvent_R, event);
}

void RLayer::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mousePressEvent_R == R_NilValue) {
    QGraphicsItem::mousePressEvent(event);
    if (mouseMoveEvent_R != R_NilValue || mouseReleaseEvent_R != R_NilValue ||
        mouseDoubleClickEvent_R != R_NilValue) {
      event->accept(); // accept this if we want other mouse events
    }    
  }
  else dispatchMouseEvent(mousePressEvent_R, event);
}

void RLayer::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseReleaseEvent_R == R_NilValue)
    QGraphicsItem::mouseReleaseEvent(event);
  else dispatchMouseEvent(mouseReleaseEvent_R, event);
}

void RLayer::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
  SEXP e, etmp, revent;
  static const char * eventNames[]
    = { "item", "pos", "screenPos", "delta", "modifiers", "view", NULL };
  
  if (wheelEvent_R == R_NilValue) {
    QGraphicsItem::wheelEvent(event);
    return;
  }
  
  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, wheelEvent_R);
  etmp = CDR(e);
  revent = allocVector(VECSXP, 6);
  SETCAR(etmp, revent);

  SET_VECTOR_ELT(revent, 0, wrapQGraphicsWidget(this));
  SET_VECTOR_ELT(revent, 1, asRPoint(event->pos()));
  SET_VECTOR_ELT(revent, 2, viewPosForEvent(event));
  SET_VECTOR_ELT(revent, 3, ScalarInteger(event->delta()));
  SET_VECTOR_ELT(revent, 4, asRModifiers(event->modifiers()));
  SET_VECTOR_ELT(revent, 5, viewForEvent(event));
  
  setAttrib(revent, R_NamesSymbol, asRStringArray(eventNames));
    
  R_tryEval(e, R_GlobalEnv, NULL);
  
  UNPROTECT(1);
}


#include <QGraphicsView>

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

  SEXP qt_qscene(void)
  {
    QGraphicsScene* scene = new QGraphicsScene(NULL);
    return wrapQGraphicsScene(scene);
  }
  
  SEXP qt_qaddWidget_QGraphicsScene(SEXP rself, SEXP ritem) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    QGraphicsWidget *item = unwrapQObject(ritem, QGraphicsWidget);
    scene->addItem(item);
    // Make sure that item is not deleted before its scene
    // FIXME: leaky -- needs to happen in itemChange() upon ItemSceneChange,
    // but that requires always using a QGraphicsWidget subclass
    addQGraphicsWidgetReference(item, scene);
    return rself;
  }

  SEXP qt_qsetBackground_QGraphicsScene(SEXP rself, SEXP rcolor) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    scene->setBackgroundBrush(QBrush(asQColor(rcolor)));
    return rself;
  }
  
  SEXP qt_qupdate_QGraphicsView(SEXP rself) {
    QGraphicsView *view = unwrapQObject(rself, QGraphicsView);
    view->scene()->update();
    view->viewport()->repaint();
    return rself;
  }

  SEXP qt_qupdate_QGraphicsScene(SEXP rself) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    QList<QGraphicsItem *> items = scene->items();
    for (int i = 0; i < items.size(); i++) {
      QGraphicsItem *item = items[i];
      QGraphicsItem::CacheMode mode = item->cacheMode();
      item->setCacheMode(QGraphicsItem::NoCache);
      item->setCacheMode(mode);
    }
    scene->update();
    return rself;
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

  SEXP qt_qviewportRect_QGraphicsView(SEXP rself) {
    QGraphicsView *view = unwrapQObject(rself, QGraphicsView);
    return asRRect(view->viewport()->rect());
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
  
  SEXP qt_qoverlay_PlotView(SEXP rself) {
    PlotView *self = unwrapQObject(rself, PlotView);
    return wrapQGraphicsScene(self->overlay());
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

static const char * keyToString(int key) {
  switch(key) {
  case Qt::Key_Escape: return "escape";
  case Qt::Key_Tab: return "tab";
  case Qt::Key_Backtab: return "backtab";
  case Qt::Key_Backspace: return "backspace";
  case Qt::Key_Return: return "return";
  case Qt::Key_Enter: return "enter";
  case Qt::Key_Insert: return "insert";
  case Qt::Key_Delete: return "delete";
  case Qt::Key_Pause: return "pause";
  case Qt::Key_Print: return "print";
  case Qt::Key_SysReq: return "sysReq";
  case Qt::Key_Clear: return "clear";
  case Qt::Key_Home: return "home";
  case Qt::Key_End: return "end";
  case Qt::Key_Left: return "left";
  case Qt::Key_Up: return "up";
  case Qt::Key_Right: return "right";
  case Qt::Key_Down: return "down";
  case Qt::Key_PageUp: return "pageUp";
  case Qt::Key_PageDown: return "pageDown";
  case Qt::Key_Shift: return "shift";
  case Qt::Key_Control: return "control";
  case Qt::Key_Meta: return "meta";
  case Qt::Key_Alt: return "alt";
  case Qt::Key_AltGr: return "altGr";
  case Qt::Key_CapsLock: return "capsLock";
  case Qt::Key_NumLock: return "numLock";
  case Qt::Key_ScrollLock: return "scrollLock";
  case Qt::Key_F1: return "f1";
  case Qt::Key_F2: return "f2";
  case Qt::Key_F3: return "f3";
  case Qt::Key_F4: return "f4";
  case Qt::Key_F5: return "f5";
  case Qt::Key_F6: return "f6";
  case Qt::Key_F7: return "f7";
  case Qt::Key_F8: return "f8";
  case Qt::Key_F9: return "f9";
  case Qt::Key_F10: return "f10";
  case Qt::Key_F11: return "f11";
  case Qt::Key_F12: return "f12";
  case Qt::Key_F13: return "f13";
  case Qt::Key_F14: return "f14";
  case Qt::Key_F15: return "f15";
  case Qt::Key_F16: return "f16";
  case Qt::Key_F17: return "f17";
  case Qt::Key_F18: return "f18";
  case Qt::Key_F19: return "f19";
  case Qt::Key_F20: return "f20";
  case Qt::Key_F21: return "f21";
  case Qt::Key_F22: return "f22";
  case Qt::Key_F23: return "f23";
  case Qt::Key_F24: return "f24";
  case Qt::Key_F25: return "f25";
  case Qt::Key_F26: return "f26";
  case Qt::Key_F27: return "f27";
  case Qt::Key_F28: return "f28";
  case Qt::Key_F29: return "f29";
  case Qt::Key_F30: return "f30";
  case Qt::Key_F31: return "f31";
  case Qt::Key_F32: return "f32";
  case Qt::Key_F33: return "f33";
  case Qt::Key_F34: return "f34";
  case Qt::Key_F35: return "f35";
  case Qt::Key_Super_L: return "super_L";
  case Qt::Key_Super_R: return "super_R";
  case Qt::Key_Menu: return "menu";
  case Qt::Key_Hyper_L: return "hyper_L";
  case Qt::Key_Hyper_R: return "hyper_R";
  case Qt::Key_Help: return "help";
  case Qt::Key_Direction_L: return "direction_L";
  case Qt::Key_Direction_R: return "direction_R";
  case Qt::Key_Space: return "space";
    //case Qt::Key_Any: return "any";
  case Qt::Key_Exclam: return "exclam";
  case Qt::Key_QuoteDbl: return "quoteDbl";
  case Qt::Key_NumberSign: return "numberSign";
  case Qt::Key_Dollar: return "dollar";
  case Qt::Key_Percent: return "percent";
  case Qt::Key_Ampersand: return "ampersand";
  case Qt::Key_Apostrophe: return "apostrophe";
  case Qt::Key_ParenLeft: return "parenLeft";
  case Qt::Key_ParenRight: return "parenRight";
  case Qt::Key_Asterisk: return "asterisk";
  case Qt::Key_Plus: return "plus";
  case Qt::Key_Comma: return "comma";
  case Qt::Key_Minus: return "minus";
  case Qt::Key_Period: return "period";
  case Qt::Key_Slash: return "slash";
  case Qt::Key_0: return "0";
  case Qt::Key_1: return "1";
  case Qt::Key_2: return "2";
  case Qt::Key_3: return "3";
  case Qt::Key_4: return "4";
  case Qt::Key_5: return "5";
  case Qt::Key_6: return "6";
  case Qt::Key_7: return "7";
  case Qt::Key_8: return "8";
  case Qt::Key_9: return "9";
  case Qt::Key_Colon: return "colon";
  case Qt::Key_Semicolon: return "semicolon";
  case Qt::Key_Less: return "less";
  case Qt::Key_Equal: return "equal";
  case Qt::Key_Greater: return "greater";
  case Qt::Key_Question: return "question";
  case Qt::Key_At: return "at";
  case Qt::Key_A: return "a";
  case Qt::Key_B: return "b";
  case Qt::Key_C: return "c";
  case Qt::Key_D: return "d";
  case Qt::Key_E: return "e";
  case Qt::Key_F: return "f";
  case Qt::Key_G: return "g";
  case Qt::Key_H: return "h";
  case Qt::Key_I: return "i";
  case Qt::Key_J: return "j";
  case Qt::Key_K: return "k";
  case Qt::Key_L: return "l";
  case Qt::Key_M: return "m";
  case Qt::Key_N: return "n";
  case Qt::Key_O: return "o";
  case Qt::Key_P: return "p";
  case Qt::Key_Q: return "q";
  case Qt::Key_R: return "r";
  case Qt::Key_S: return "s";
  case Qt::Key_T: return "t";
  case Qt::Key_U: return "u";
  case Qt::Key_V: return "v";
  case Qt::Key_W: return "w";
  case Qt::Key_X: return "x";
  case Qt::Key_Y: return "y";
  case Qt::Key_Z: return "z";
  case Qt::Key_BracketLeft: return "bracketLeft";
  case Qt::Key_Backslash: return "backslash";
  case Qt::Key_BracketRight: return "bracketRight";
  case Qt::Key_AsciiCircum: return "asciiCircum";
  case Qt::Key_Underscore: return "underscore";
  case Qt::Key_QuoteLeft: return "quoteLeft";
  case Qt::Key_BraceLeft: return "braceLeft";
  case Qt::Key_Bar: return "bar";
  case Qt::Key_BraceRight: return "braceRight";
  case Qt::Key_AsciiTilde: return "asciiTilde";
  case Qt::Key_nobreakspace: return "nobreakspace";
  case Qt::Key_exclamdown: return "exclamdown";
  case Qt::Key_cent: return "cent";
  case Qt::Key_sterling: return "sterling";
  case Qt::Key_currency: return "currency";
  case Qt::Key_yen: return "yen";
  case Qt::Key_brokenbar: return "brokenbar";
  case Qt::Key_section: return "section";
  case Qt::Key_diaeresis: return "diaeresis";
  case Qt::Key_copyright: return "copyright";
  case Qt::Key_ordfeminine: return "ordfeminine";
  case Qt::Key_guillemotleft: return "guillemotleft";
  case Qt::Key_notsign: return "notsign";
  case Qt::Key_hyphen: return "hyphen";
  case Qt::Key_registered: return "registered";
  case Qt::Key_macron: return "macron";
  case Qt::Key_degree: return "degree";
  case Qt::Key_plusminus: return "plusminus";
  case Qt::Key_twosuperior: return "twosuperior";
  case Qt::Key_threesuperior: return "threesuperior";
  case Qt::Key_acute: return "acute";
  case Qt::Key_mu: return "mu";
  case Qt::Key_paragraph: return "paragraph";
  case Qt::Key_periodcentered: return "periodcentered";
  case Qt::Key_cedilla: return "cedilla";
  case Qt::Key_onesuperior: return "onesuperior";
  case Qt::Key_masculine: return "masculine";
  case Qt::Key_guillemotright: return "guillemotright";
  case Qt::Key_onequarter: return "onequarter";
  case Qt::Key_onehalf: return "onehalf";
  case Qt::Key_threequarters: return "threequarters";
  case Qt::Key_questiondown: return "questiondown";
  case Qt::Key_Agrave: return "agrave";
  case Qt::Key_Aacute: return "aacute";
  case Qt::Key_Acircumflex: return "acircumflex";
  case Qt::Key_Atilde: return "atilde";
  case Qt::Key_Adiaeresis: return "adiaeresis";
  case Qt::Key_Aring: return "aring";
  case Qt::Key_AE: return "aE";
  case Qt::Key_Ccedilla: return "ccedilla";
  case Qt::Key_Egrave: return "egrave";
  case Qt::Key_Eacute: return "eacute";
  case Qt::Key_Ecircumflex: return "ecircumflex";
  case Qt::Key_Ediaeresis: return "ediaeresis";
  case Qt::Key_Igrave: return "igrave";
  case Qt::Key_Iacute: return "iacute";
  case Qt::Key_Icircumflex: return "icircumflex";
  case Qt::Key_Idiaeresis: return "idiaeresis";
  case Qt::Key_ETH: return "eTH";
  case Qt::Key_Ntilde: return "ntilde";
  case Qt::Key_Ograve: return "ograve";
  case Qt::Key_Oacute: return "oacute";
  case Qt::Key_Ocircumflex: return "ocircumflex";
  case Qt::Key_Otilde: return "otilde";
  case Qt::Key_Odiaeresis: return "odiaeresis";
  case Qt::Key_multiply: return "multiply";
  case Qt::Key_Ooblique: return "ooblique";
  case Qt::Key_Ugrave: return "ugrave";
  case Qt::Key_Uacute: return "uacute";
  case Qt::Key_Ucircumflex: return "ucircumflex";
  case Qt::Key_Udiaeresis: return "udiaeresis";
  case Qt::Key_Yacute: return "yacute";
  case Qt::Key_THORN: return "tHORN";
  case Qt::Key_ssharp: return "ssharp";
  case Qt::Key_division: return "division";
  case Qt::Key_ydiaeresis: return "ydiaeresis";
  case Qt::Key_Multi_key: return "multi_key";
  case Qt::Key_Codeinput: return "codeinput";
  case Qt::Key_SingleCandidate: return "singleCandidate";
  case Qt::Key_MultipleCandidate: return "multipleCandidate";
  case Qt::Key_PreviousCandidate: return "previousCandidate";
  case Qt::Key_Mode_switch: return "mode_switch";
  case Qt::Key_Kanji: return "kanji";
  case Qt::Key_Muhenkan: return "muhenkan";
  case Qt::Key_Henkan: return "henkan";
  case Qt::Key_Romaji: return "romaji";
  case Qt::Key_Hiragana: return "hiragana";
  case Qt::Key_Katakana: return "katakana";
  case Qt::Key_Hiragana_Katakana: return "hiragana_Katakana";
  case Qt::Key_Zenkaku: return "zenkaku";
  case Qt::Key_Hankaku: return "hankaku";
  case Qt::Key_Zenkaku_Hankaku: return "zenkaku_Hankaku";
  case Qt::Key_Touroku: return "touroku";
  case Qt::Key_Massyo: return "massyo";
  case Qt::Key_Kana_Lock: return "kana_Lock";
  case Qt::Key_Kana_Shift: return "kana_Shift";
  case Qt::Key_Eisu_Shift: return "eisu_Shift";
  case Qt::Key_Eisu_toggle: return "eisu_toggle";
  case Qt::Key_Hangul: return "hangul";
  case Qt::Key_Hangul_Start: return "hangul_Start";
  case Qt::Key_Hangul_End: return "hangul_End";
  case Qt::Key_Hangul_Hanja: return "hangul_Hanja";
  case Qt::Key_Hangul_Jamo: return "hangul_Jamo";
  case Qt::Key_Hangul_Romaja: return "hangul_Romaja";
  case Qt::Key_Hangul_Jeonja: return "hangul_Jeonja";
  case Qt::Key_Hangul_Banja: return "hangul_Banja";
  case Qt::Key_Hangul_PreHanja: return "hangul_PreHanja";
  case Qt::Key_Hangul_PostHanja: return "hangul_PostHanja";
  case Qt::Key_Hangul_Special: return "hangul_Special";
  case Qt::Key_Dead_Grave: return "dead_Grave";
  case Qt::Key_Dead_Acute: return "dead_Acute";
  case Qt::Key_Dead_Circumflex: return "dead_Circumflex";
  case Qt::Key_Dead_Tilde: return "dead_Tilde";
  case Qt::Key_Dead_Macron: return "dead_Macron";
  case Qt::Key_Dead_Breve: return "dead_Breve";
  case Qt::Key_Dead_Abovedot: return "dead_Abovedot";
  case Qt::Key_Dead_Diaeresis: return "dead_Diaeresis";
  case Qt::Key_Dead_Abovering: return "dead_Abovering";
  case Qt::Key_Dead_Doubleacute: return "dead_Doubleacute";
  case Qt::Key_Dead_Caron: return "dead_Caron";
  case Qt::Key_Dead_Cedilla: return "dead_Cedilla";
  case Qt::Key_Dead_Ogonek: return "dead_Ogonek";
  case Qt::Key_Dead_Iota: return "dead_Iota";
  case Qt::Key_Dead_Voiced_Sound: return "dead_Voiced_Sound";
  case Qt::Key_Dead_Semivoiced_Sound: return "dead_Semivoiced_Sound";
  case Qt::Key_Dead_Belowdot: return "dead_Belowdot";
  case Qt::Key_Dead_Hook: return "dead_Hook";
  case Qt::Key_Dead_Horn: return "dead_Horn";
  case Qt::Key_Back: return "back";
  case Qt::Key_Forward: return "forward";
  case Qt::Key_Stop: return "stop";
  case Qt::Key_Refresh: return "refresh";
  case Qt::Key_VolumeDown: return "volumeDown";
  case Qt::Key_VolumeMute: return "volumeMute";
  case Qt::Key_VolumeUp: return "volumeUp";
  case Qt::Key_BassBoost: return "bassBoost";
  case Qt::Key_BassUp: return "bassUp";
  case Qt::Key_BassDown: return "bassDown";
  case Qt::Key_TrebleUp: return "trebleUp";
  case Qt::Key_TrebleDown: return "trebleDown";
  case Qt::Key_MediaPlay: return "mediaPlay";
  case Qt::Key_MediaStop: return "mediaStop";
  case Qt::Key_MediaPrevious: return "mediaPrevious";
  case Qt::Key_MediaNext: return "mediaNext";
  case Qt::Key_MediaRecord: return "mediaRecord";
  case Qt::Key_HomePage: return "homePage";
  case Qt::Key_Favorites: return "favorites";
  case Qt::Key_Search: return "search";
  case Qt::Key_Standby: return "standby";
  case Qt::Key_OpenUrl: return "openUrl";
  case Qt::Key_LaunchMail: return "launchMail";
  case Qt::Key_LaunchMedia: return "launchMedia";
  case Qt::Key_Launch0: return "launch0";
  case Qt::Key_Launch1: return "launch1";
  case Qt::Key_Launch2: return "launch2";
  case Qt::Key_Launch3: return "launch3";
  case Qt::Key_Launch4: return "launch4";
  case Qt::Key_Launch5: return "launch5";
  case Qt::Key_Launch6: return "launch6";
  case Qt::Key_Launch7: return "launch7";
  case Qt::Key_Launch8: return "launch8";
  case Qt::Key_Launch9: return "launch9";
  case Qt::Key_LaunchA: return "launchA";
  case Qt::Key_LaunchB: return "launchB";
  case Qt::Key_LaunchC: return "launchC";
  case Qt::Key_LaunchD: return "launchD";
  case Qt::Key_LaunchE: return "launchE";
  case Qt::Key_LaunchF: return "launchF";
  case Qt::Key_MediaLast: return "mediaLast";
  case Qt::Key_unknown: return "unknown";
  case Qt::Key_Call: return "call";
  case Qt::Key_Context1: return "context1";
  case Qt::Key_Context2: return "context2";
  case Qt::Key_Context3: return "context3";
  case Qt::Key_Context4: return "context4";
  case Qt::Key_Flip: return "flip";
  case Qt::Key_Hangup: return "hangup";
  case Qt::Key_No: return "no";
  case Qt::Key_Select: return "select";
  case Qt::Key_Yes: return "yes";
  case Qt::Key_Execute: return "execute";
  case Qt::Key_Printer: return "printer";
  case Qt::Key_Play: return "play";
  case Qt::Key_Sleep: return "sleep";
  case Qt::Key_Zoom: return "zoom";
  case Qt::Key_Cancel: return "cancel";
  default: return "unknown";
  }
}
