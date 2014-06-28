#ifndef PLOTVIEW_H
#define PLOTVIEW_H 1

#include <QGraphicsView>

#include "Layer.hpp"


/* Specialized GraphicsView for plotting.
   - automatic rescaling
   - 'overlay' as a scene drawn in viewport coordinates in the foreground.
*/
namespace Qanviz {

  class PlotView;

  /* A per-view collection of items to be rendered in device
     coordinates, in the foreground. Does not accept events. */
  class OverlayScene : public QGraphicsScene {
    Q_OBJECT
    
  private:
    QGraphicsView *_view;
  
  public:
    QGraphicsView * view() { return _view; }

  protected:
    OverlayScene(QGraphicsView *view) : QGraphicsScene(view), _view(view) { }
    void drawOverlay(QPainter *painter, QRectF exposed);

    friend class PlotView;
  };

  class PlotView : public QGraphicsView {
    Q_OBJECT

  public:

    static PlotView *paintingView(QGraphicsScene *scene);
    
    enum RescaleMode {
      None,
      WidgetGeometry, /* activates layout, single view only */
      ViewTransform /* for rescaling with multiple views - useful? */
    };
    
    PlotView(QGraphicsScene *scene, QWidget *parent = 0,
             RescaleMode rescaleMode = WidgetGeometry, bool opengl = true);

    //virtual ~PlotView();
    
    void setRescaleMode(RescaleMode rescaleMode) {
      if (rescaleMode > None) {
        // NOTE: seems only to have effect after widget is visible
        Qt::ScrollBarPolicy policy = Qt::ScrollBarAlwaysOn;
        setHorizontalScrollBarPolicy(policy);
        setVerticalScrollBarPolicy(policy);
        if (rescaleMode == ViewTransform)
          setResizeAnchor(QGraphicsView::AnchorViewCenter);
        else setResizeAnchor(QGraphicsView::NoAnchor);
      }
      _rescaleMode = rescaleMode;
    }
    RescaleMode rescaleMode() const {
      return _rescaleMode;
    }

    OverlayScene *overlay() {
      return _overlay;
    }
    
    bool isPainting() {
      return _isPainting;
    }

    void setOpenGL(bool opengl);

    // When rescaleMode == Geometry, we are responsible for the scene
    // geometry. Sometimes, like when the transform changes, we need
    // to recalcuate it.
    void updateSceneGeometry();
    
  protected:

    void resizeEvent (QResizeEvent * event);
    void paintEvent (QPaintEvent * event);
    
    void drawForeground(QPainter *painter, const QRectF &rect);
    //void drawBackground(QPainter *painter, const QRectF &rect);
    
  private:
        
    RescaleMode _rescaleMode;
    QSizeF oldSize;
    OverlayScene *_overlay;
    bool _isPainting;

    void updateSceneGeometry(QGraphicsScene *scene);
    void drawMetaScene(OverlayScene *scene, QPainter *painter, QRectF rect);
  };
}

#endif
