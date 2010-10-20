#ifndef RLAYER_H
#define RLAYER_H

#include "Layer.hpp"
#include "Rinternals.h"

class Painter;

namespace Qanviz {
  class RLayer : public Layer {
    Q_OBJECT
    
  private:
    SEXP paintEvent_R, keyPressEvent_R, keyReleaseEvent_R,
      mouseDoubleClickEvent_R, mouseMoveEvent_R, mousePressEvent_R,
      mouseReleaseEvent_R, wheelEvent_R, hoverMoveEvent_R,
      hoverEnterEvent_R, hoverLeaveEvent_R, contextMenuEvent_R,
      dragEnterEvent_R, dragLeaveEvent_R, dragMoveEvent_R,
      dropEvent_R, focusInEvent_R, focusOutEvent_R, sizeHint_R;

    void dispatchKeyEvent(SEXP closure, QKeyEvent *event);
    void dispatchMouseEvent(SEXP closure, QGraphicsSceneMouseEvent *event);
    void dispatchHoverEvent(SEXP closure, QGraphicsSceneHoverEvent *event);
    
  public:
    // FIXME: callbacks should probably be accessed via
    // setters/getters, and the constructor simplified to 'parent'
    RLayer(QGraphicsItem *parent,
           SEXP paintEvent, SEXP keyPressEvent, SEXP keyReleaseEvent,
           SEXP mouseDoubleClickEvent, SEXP mouseMoveEvent,
           SEXP mousePressEvent, SEXP mouseReleaseEvent, SEXP wheelEvent,
           SEXP hoverMoveEvent, SEXP hoverEnterEvent,
           SEXP hoverLeaveEvent, SEXP contextMenuEvent,
           SEXP dragEnterEvent, SEXP dragLeaveEvent,
           SEXP dragMoveEvent, SEXP dropEvent,
           SEXP focusInEvent, SEXP focusOutEvent,
           SEXP sizeHint);

    virtual ~RLayer();
    
    void paintPlot(Painter *painter, QRectF exposed);
    
  protected:
    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void wheelEvent ( QGraphicsSceneWheelEvent * event );
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    void keyReleaseEvent ( QKeyEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event );
    void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
    void dragLeaveEvent ( QGraphicsSceneDragDropEvent * event );
    void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
    void dropEvent ( QGraphicsSceneDragDropEvent * event );
    void focusInEvent ( QFocusEvent * event );
    void focusOutEvent ( QFocusEvent * event );
    QSizeF sizeHint (Qt::SizeHint hint, const QSizeF &constraint = QSizeF());

  };
}

extern "C" SEXP qanviz_RLayer(SEXP args);

#endif
