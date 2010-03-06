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
  if (keyPressEvent != R_NilValue || keyReleaseEvent != R_NilValue)
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
  QList<QString> painterClasses;
  painterClasses.append("Painter");
  
  if (paintEvent_R == R_NilValue)
    return; // not painting (just for layout)

  PROTECT(e = allocVector(LANGSXP, 3 + wantsExposed));
  SETCAR(e, paintEvent_R);
  etmp = CDR(e);
  SETCAR(etmp, wrapSmoke(this, Layer, false));
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

template<typename T> void dispatchEvent(SEXP closure, T event) {
  SEXP e;

  if (closure == R_NilValue) {
    return;
  }
  
  PROTECT(e = allocVector(LANGSXP, 2));
  SETCAR(e, closure);
  SETCADR(e, wrapSmoke(event, T, false));
  
  R_tryEval(e, R_GlobalEnv, NULL);
  
  UNPROTECT(1);
}

void RLayer::keyPressEvent ( QKeyEvent * event ) {
  dispatchEvent(keyPressEvent_R, event);
}

void RLayer::keyReleaseEvent ( QKeyEvent * event ) {
  dispatchEvent(keyReleaseEvent_R, event);
}

void RLayer::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseDoubleClickEvent_R == R_NilValue)
    QGraphicsItem::mouseDoubleClickEvent(event);
  else dispatchEvent(mouseDoubleClickEvent_R, event);
}

/* NOTE: this is really more like a 'dragging' event */
void RLayer::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseMoveEvent_R == R_NilValue)  
    QGraphicsItem::mouseMoveEvent(event);
  else dispatchEvent(mouseMoveEvent_R, event);
}

void RLayer::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mousePressEvent_R == R_NilValue) {
    QGraphicsItem::mousePressEvent(event);
    if (mouseMoveEvent_R != R_NilValue || mouseReleaseEvent_R != R_NilValue ||
        mouseDoubleClickEvent_R != R_NilValue) {
      event->accept(); // accept this if we want other mouse events
    }    
  }
  else dispatchEvent(mousePressEvent_R, event);
}

void RLayer::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
  if (mouseReleaseEvent_R == R_NilValue)
    QGraphicsItem::mouseReleaseEvent(event);
  else dispatchEvent(mouseReleaseEvent_R, event);
}

void RLayer::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
  dispatchEvent(wheelEvent_R, event);
}

void RLayer::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverEnterEvent_R == R_NilValue)  
    QGraphicsItem::hoverEnterEvent(event);
  else dispatchEvent(hoverEnterEvent_R, event);
}

void RLayer::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverLeaveEvent_R == R_NilValue)  
    QGraphicsItem::hoverLeaveEvent(event);
  else dispatchEvent(hoverLeaveEvent_R, event);
}

void RLayer::hoverMoveEvent ( QGraphicsSceneHoverEvent * event ) {
  if (hoverMoveEvent_R == R_NilValue)  
    QGraphicsItem::hoverMoveEvent(event);
  else dispatchEvent(hoverMoveEvent_R, event);
}

void RLayer::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event ) {
  if (contextMenuEvent_R == R_NilValue)  
    QGraphicsItem::contextMenuEvent(event);
  else dispatchEvent(contextMenuEvent_R, event);
}

void RLayer::dragEnterEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragEnterEvent_R == R_NilValue)  
    QGraphicsItem::dragEnterEvent(event);
  else dispatchEvent(dragEnterEvent_R, event);
}  

void RLayer::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragLeaveEvent_R == R_NilValue)  
    QGraphicsItem::dragLeaveEvent(event);
  else dispatchEvent(dragLeaveEvent_R, event);
}

void RLayer::dragMoveEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dragMoveEvent_R == R_NilValue)  
    QGraphicsItem::dragMoveEvent(event);
  else dispatchEvent(dragMoveEvent_R, event);
}

void RLayer::dropEvent ( QGraphicsSceneDragDropEvent * event ) {
  if (dropEvent_R == R_NilValue)  
    QGraphicsItem::dropEvent(event);
  else dispatchEvent(dropEvent_R, event);
}

void RLayer::focusInEvent ( QFocusEvent * event ) {
  if (focusInEvent_R == R_NilValue)  
    QGraphicsItem::focusInEvent(event);
  else dispatchEvent(focusInEvent_R, event);
}

void RLayer::focusOutEvent ( QFocusEvent * event ) {
  if (focusOutEvent_R == R_NilValue)  
    QGraphicsItem::focusOutEvent(event);
  else dispatchEvent(focusOutEvent_R, event);
}

QSizeF RLayer::sizeHint ( Qt::SizeHint hint, QSizeF &constraint ) {
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
