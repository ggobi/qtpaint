#include <QGraphicsSceneMouseEvent>

#include "RLayer.hpp"
#include <qtbase.h>

/* This class allows the R user to implement a custom Layer without a
   separate class. While this could be done purely within R, RLayer
   probably has much better performance. There is more overhead to
   virtual callbacks in R, and here we completely avoid calling R for
   unhandled events. The optimization may be premature, but since we
   already have this class, let's keep it for now. Maintenance wise,
   C++ is more than bearable for this simple task. */

using namespace Qanviz;

#define PRESERVE_CALLBACK(x) \
  if (x ## _R != R_NilValue) R_PreserveObject(x ## _R)
#define RELEASE_CALLBACK(x) if (x ## _R != R_NilValue) R_ReleaseObject(x ## _R)

void invalidatePtr(SEXP ptr) {
  setAttrib(ptr, R_ClassSymbol, mkString("<invalid>"));
}

RLayer::RLayer(QGraphicsItem *parent,
               SEXP paintEvent, SEXP keyPressEvent, SEXP keyReleaseEvent,
               SEXP mouseDoubleClickEvent, SEXP mouseMoveEvent,
               SEXP mousePressEvent, SEXP mouseReleaseEvent, SEXP wheelEvent,
               SEXP hoverMoveEvent, SEXP hoverEnterEvent,
               SEXP hoverLeaveEvent, SEXP contextMenuEvent,
               SEXP dragEnterEvent, SEXP dragLeaveEvent,
               SEXP dragMoveEvent, SEXP dropEvent,
               SEXP focusInEvent, SEXP focusOutEvent,               
               SEXP sizeHint)
  : Layer(parent), paintEvent_R(paintEvent), keyPressEvent_R(keyPressEvent),
    keyReleaseEvent_R(keyReleaseEvent),
    mouseDoubleClickEvent_R(mouseDoubleClickEvent),
    mouseMoveEvent_R(mouseMoveEvent), mousePressEvent_R(mousePressEvent),
    mouseReleaseEvent_R(mouseReleaseEvent), wheelEvent_R(wheelEvent),
    hoverMoveEvent_R(hoverMoveEvent), hoverEnterEvent_R(hoverEnterEvent),
    hoverLeaveEvent_R(hoverLeaveEvent), contextMenuEvent_R(contextMenuEvent),
    dragEnterEvent_R(dragEnterEvent), dragLeaveEvent_R(dragLeaveEvent),
    dragMoveEvent_R(dragMoveEvent), dropEvent_R(dropEvent),
    focusInEvent_R(focusInEvent), focusOutEvent_R(focusOutEvent),
    sizeHint_R(sizeHint)
{
  if (keyPressEvent != R_NilValue || keyReleaseEvent != R_NilValue ||
      focusInEvent != R_NilValue || focusOutEvent != R_NilValue)
    setFocusPolicy(Qt::StrongFocus);
  if (hoverMoveEvent != R_NilValue || hoverEnterEvent != R_NilValue ||
      hoverLeaveEvent != R_NilValue)
    setAcceptHoverEvents(true);
  if (dragEnterEvent != R_NilValue || dragMoveEvent != R_NilValue ||
      dragLeaveEvent != R_NilValue || dropEvent != R_NilValue)
    setAcceptDrops(true);
#if QT_VERSION >= 0x40600
  if (paintEvent == R_NilValue)
    setFlag(QGraphicsItem::ItemHasNoContents);
  if (length(FORMALS(paintEvent)) > 2) // user requests exposed rect
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
#endif
  if (mousePressEvent == R_NilValue && mouseMoveEvent == R_NilValue &&
      mouseReleaseEvent == R_NilValue && mouseDoubleClickEvent == R_NilValue)
    setAcceptedMouseButtons(0);
  if (sizeHint != R_NilValue)
    updateGeometry();
  /* preserve every callback */
  PRESERVE_CALLBACK(paintEvent);
  PRESERVE_CALLBACK(keyPressEvent);
  PRESERVE_CALLBACK(keyReleaseEvent);
  PRESERVE_CALLBACK(mouseDoubleClickEvent);
  PRESERVE_CALLBACK(mouseMoveEvent);
  PRESERVE_CALLBACK(mousePressEvent);
  PRESERVE_CALLBACK(mouseReleaseEvent);
  PRESERVE_CALLBACK(wheelEvent);
  PRESERVE_CALLBACK(hoverMoveEvent);
  PRESERVE_CALLBACK(hoverEnterEvent);
  PRESERVE_CALLBACK(hoverLeaveEvent);
  PRESERVE_CALLBACK(contextMenuEvent);
  PRESERVE_CALLBACK(dragEnterEvent);
  PRESERVE_CALLBACK(dragLeaveEvent);
  PRESERVE_CALLBACK(dragMoveEvent);
  PRESERVE_CALLBACK(dropEvent);
  PRESERVE_CALLBACK(focusInEvent);
  PRESERVE_CALLBACK(focusOutEvent);
  PRESERVE_CALLBACK(sizeHint);
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
  RELEASE_CALLBACK(hoverMoveEvent);
  RELEASE_CALLBACK(hoverEnterEvent);
  RELEASE_CALLBACK(hoverLeaveEvent);
  RELEASE_CALLBACK(contextMenuEvent);
  RELEASE_CALLBACK(dragEnterEvent);
  RELEASE_CALLBACK(dragLeaveEvent);
  RELEASE_CALLBACK(dragMoveEvent);
  RELEASE_CALLBACK(dropEvent);
  RELEASE_CALLBACK(focusInEvent);
  RELEASE_CALLBACK(focusOutEvent);
  RELEASE_CALLBACK(sizeHint);
}

void RLayer::paintPlot(Painter *painter, QRectF exposed)
{
  SEXP e, etmp, rpainter;
  bool wantsExposed = length(FORMALS(paintEvent_R)) > 2;
  QList<QByteArray> painterClasses;
  painterClasses.append("Painter");
  
  if (paintEvent_R == R_NilValue)
    return; // not painting (just for layout)

  PROTECT(e = allocVector(LANGSXP, 3 + wantsExposed));
  SETCAR(e, paintEvent_R);
  etmp = CDR(e);
  SETCAR(etmp, wrapSmoke(this, Qanviz::RLayer, false));
  etmp = CDR(etmp);
  rpainter = wrapPointer(painter, painterClasses, NULL);
  SETCAR(etmp, rpainter);
  if (wantsExposed) {
    etmp = CDR(etmp);
    SETCAR(etmp, wrapSmokeCopy(exposed, QRectF));
  }
  R_tryEval(e, R_GlobalEnv, NULL);

  /* We cannot guarantee that these pointers will be valid */
  invalidatePtr(rpainter);

  UNPROTECT(1);
}

template<typename T> void dispatchEvent(RLayer *layer, SEXP closure, T event,
                                        const char *eventClass)
{
  SEXP e;

  if (closure == R_NilValue) {
    return;
  }

  e = lang3(closure, wrapSmoke(layer, Qanviz::RLayer, false),
            _wrapSmoke(event, eventClass, false));
  
  R_tryEval(e, R_GlobalEnv, NULL);
}

void RLayer::keyPressEvent ( QKeyEvent * event ) {
  if (keyPressEvent_R == R_NilValue)
    QGraphicsItem::keyPressEvent(event);
  else dispatchEvent(this, keyPressEvent_R, event, "QKeyEvent");
}

void RLayer::keyReleaseEvent ( QKeyEvent * event ) {
  if (keyReleaseEvent_R == R_NilValue)
    QGraphicsItem::keyReleaseEvent(event);
  else dispatchEvent(this, keyReleaseEvent_R, event, "QKeyEvent");
}

void RLayer::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseDoubleClickEvent_R == R_NilValue)
    QGraphicsItem::mouseDoubleClickEvent(event);
  else dispatchEvent(this, mouseDoubleClickEvent_R, event,
                     "QGraphicsSceneMouseEvent");
}

/* NOTE: this is really more like a 'dragging' event */
void RLayer::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseMoveEvent_R == R_NilValue)  
    QGraphicsItem::mouseMoveEvent(event);
  else dispatchEvent(this, mouseMoveEvent_R, event, "QGraphicsSceneMouseEvent");
}

void RLayer::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mousePressEvent_R == R_NilValue) {
    QGraphicsItem::mousePressEvent(event);
    if (mouseMoveEvent_R != R_NilValue || mouseReleaseEvent_R != R_NilValue ||
        mouseDoubleClickEvent_R != R_NilValue) {
      event->accept(); // accept this if we want other mouse events
    }    
  }
  else dispatchEvent(this, mousePressEvent_R, event,
                     "QGraphicsSceneMouseEvent");
}

void RLayer::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseReleaseEvent_R == R_NilValue)
    QGraphicsItem::mouseReleaseEvent(event);
  else dispatchEvent(this, mouseReleaseEvent_R, event,
                     "QGraphicsSceneMouseEvent");
}

void RLayer::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
  if (wheelEvent_R == R_NilValue)
    QGraphicsItem::wheelEvent(event);
  else dispatchEvent(this, wheelEvent_R, event, "QGraphicsSceneWheelEvent");
}

void RLayer::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverEnterEvent_R == R_NilValue)
    Layer::hoverEnterEvent(event);
  else dispatchEvent(this, hoverEnterEvent_R, event,
                     "QGraphicsSceneHoverEvent");
}

void RLayer::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverLeaveEvent_R == R_NilValue)
    Layer::hoverLeaveEvent(event);
  else dispatchEvent(this, hoverLeaveEvent_R, event,
                     "QGraphicsSceneHoverEvent");
}

void RLayer::hoverMoveEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverMoveEvent_R == R_NilValue)  
    QGraphicsItem::hoverMoveEvent(event);
  else dispatchEvent(this, hoverMoveEvent_R, event, "QGraphicsSceneHoverEvent");
}

void RLayer::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event ) {
  if (contextMenuEvent_R == R_NilValue)  
    QGraphicsItem::contextMenuEvent(event);
  else dispatchEvent(this, contextMenuEvent_R, event,
                     "QGraphicsSceneContextMenuEvent");
}

void RLayer::dragEnterEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragEnterEvent_R == R_NilValue)  
    QGraphicsItem::dragEnterEvent(event);
  else dispatchEvent(this, dragEnterEvent_R, event,
                     "QGraphicsSceneDragDropEvent");
}  

void RLayer::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragLeaveEvent_R == R_NilValue)  
    QGraphicsItem::dragLeaveEvent(event);
  else dispatchEvent(this, dragLeaveEvent_R, event,
                     "QGraphicsSceneDragDropEvent");
}

void RLayer::dragMoveEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragMoveEvent_R == R_NilValue)  
    QGraphicsItem::dragMoveEvent(event);
  else dispatchEvent(this, dragMoveEvent_R, event,
                     "QGraphicsSceneDragDropEvent");
}

void RLayer::dropEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dropEvent_R == R_NilValue)  
    QGraphicsItem::dropEvent(event);
  else dispatchEvent(this, dropEvent_R, event, "QGraphicsSceneDragDropEvent");
}

void RLayer::focusInEvent ( QFocusEvent * event ) {
  if (focusInEvent_R == R_NilValue)  
    Layer::focusInEvent(event);
  else dispatchEvent(this, focusInEvent_R, event, "QFocusEvent");
}

void RLayer::focusOutEvent ( QFocusEvent * event ) {
  if (focusOutEvent_R == R_NilValue)  
    Layer::focusOutEvent(event);
  else dispatchEvent(this, focusOutEvent_R, event, "QFocusEvent");
}

QSizeF RLayer::sizeHint ( Qt::SizeHint hint, const QSizeF &constraint ) {
  SEXP e, etmp, ans;
  if (sizeHint_R == R_NilValue) {
    return QGraphicsWidget::sizeHint(hint, constraint);
  }
  
  PROTECT(e = allocVector(LANGSXP, 3));
  SETCAR(e, sizeHint_R);
  etmp = CDR(e);
  SETCAR(etmp, ScalarInteger(hint));
  etmp = CDR(e);
  SETCAR(etmp, wrapSmokeCopy(constraint, QSizeF));
  ans = R_tryEval(e, R_GlobalEnv, NULL);

  UNPROTECT(1);

  return(*unwrapSmoke(ans, QSizeF));
}

#include <qanviz_smoke.h>

/* This is a hack that lets us pass SEXP's directly to RLayer() */
SEXP qanviz_RLayer(SEXP args)
{ 
  Smoke::ModuleIndex meth =
    qanviz_Smoke->findMethod("Qanviz::RLayer", "RLayer#???????????????????");
  meth.index = meth.smoke->methodMaps[meth.index].method;
  return invokeSmokeMethod(meth.smoke, meth.index, NULL, args);
}
