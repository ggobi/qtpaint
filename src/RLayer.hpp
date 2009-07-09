#ifndef RLAYER_H
#define RLAYER_H

#include "Layer.hpp"
#include "Rinternals.h"

namespace QViz {
  class RLayer : public Layer {
    Q_OBJECT
    
  private:
    SEXP paintEvent_R, keyPressEvent_R, keyReleaseEvent_R,
      mouseDoubleClickEvent_R, mouseMoveEvent_R, mousePressEvent_R,
      mouseReleaseEvent_R, wheelEvent_R, sizeHint_R;

    void dispatchKeyEvent(SEXP closure, QKeyEvent *event);
    void dispatchMouseEvent(SEXP closure, QGraphicsSceneMouseEvent *event);
    void dispatchHoverEvent(SEXP closure, QGraphicsSceneHoverEvent *event);
    
  public:

    RLayer(SEXP paintEvent, SEXP keyPressEvent, SEXP keyReleaseEvent,
           SEXP mouseDoubleClickEvent, SEXP mouseMoveEvent,
           SEXP mousePressEvent, SEXP mouseReleaseEvent, SEXP wheelEvent,
           SEXP sizeHint);

    virtual ~RLayer();
    
    void paintPlot(Painter *painter, QRectF exposed);
    QSizeF sizeHint (Qt::SizeHint hint, QSizeF &constraint);
    
  protected:
    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void wheelEvent ( QGraphicsSceneWheelEvent * event );
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    void keyReleaseEvent ( QKeyEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    
  };
}

#endif
