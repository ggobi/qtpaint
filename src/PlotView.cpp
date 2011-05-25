#include <QResizeEvent>
#include <QStyleOptionGraphicsItem>
#include <QScrollBar>
#include <QtCore/qmath.h>

#ifdef QT_OPENGL_LIB
#include <QGLWidget>
#endif

#include "PlotView.hpp"

using namespace Qanviz;

/* In the QGraphicsView framework, the each layer/geom of a plot is an
   item, and the overall plot, possibly including multiple facets, is
   the scene. A scene can have multiple views, where each view
   presumably has a different viewport transform (pan/zoom), but what
   role do multiple pannable and zoomable views play in statistical
   graphics?

   This depends on the plot layout method, for which there are at
   least two different models:

   - Disjoint: Just like R graphics, each part of a plot (the actual
     plot, the axes, title, etc) are all geometrically disjoint and
     are represented by separate items. We handle this through
     QGraphicsLayout. As in R, one can zoom by resizing the plot,
     which should change the "root" item geometry and activate the
     layout. Multiple views would really only make sense if one could
     zoom beyond the confines of the plot. There are two
     complications: (1) Items would become excluded, which would often
     be undesirable, such as when axes are hidden. There are cases
     though when one wants items to be excluded. (2) The layout would
     no longer be "packed" for fixed layers, like axes. Of course,
     fixed-layers could be un-fixed (as one would no longer require
     publication quality), but they still might be wasting space.

     For multiple views to work, they need different
     transforms, which does not seem possible if zooming occurs at the
     scene-level, as above. The zooming must happen at the view
     transform.
     
   - Overlay: Like GGobi scatterplots, all items are plots, with a
     stack of children (geoms) and various view overlays
     (guides). This way, the plot can be zoomed beyond the window
     without losing guides or wasting space, so multiple views make
     sense. Like the layout method, resizing the window should
     rescale/zoom the plot, but it cannot affect the geometry of the
     items, as that would affect scene coordinates. Thus, the view
     transform needs to be adjusted. Note that GGobi has never
     supported multiple views in this sense, so there may not be a
     strong use case for this model. Multiple view applications like
     Cytoscape seem not to rescale on resize.

   This class attempts to address these use cases, by providing
   overlays and automatic rescaling on resize. Overlays are just
   layers that draw in viewport coordinates. There are two rescale
   modes: synchronization of item and viewport geometry, and scaling
   by view transform.
   
   Side note: There is also the notion of a physical zoom, like a
   magnifying glass. We could do this by rendering to a QPicture and
   playing it to a transformed QPainter. Like how Graham showed in
   Bremen.
*/

PlotView::PlotView(QGraphicsScene *scene, QWidget *parent,
                   RescaleMode rescaleMode, bool opengl)
  : QGraphicsView(scene, parent), _overlay(new OverlayScene(this))
{
  setOpenGL(opengl);
  setRescaleMode(rescaleMode);
  setFrameStyle(QFrame::NoFrame);
  connect(_overlay, SIGNAL(changed(QList<QRectF>)), this,
          SLOT(update()));
}

void PlotView::resizeEvent (QResizeEvent * event)
{
  QGraphicsView::resizeEvent(event);
  if (_rescaleMode > None) {
    // initial size event, make sure scrollbars are on when needed
    if (event->oldSize().width() < 0) {
      Qt::ScrollBarPolicy policy = _rescaleMode == ViewTransform ?
        Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff;
      setHorizontalScrollBarPolicy(policy);
      setVerticalScrollBarPolicy(policy);
      QTransform identity;
      setTransform(identity);
    }
    if (_rescaleMode == ViewTransform) {
      QSizeF size = viewport()->size();
      if (event->oldSize().width() < 0) {
        fitInView(sceneRect());
        oldSize = viewport()->size();
      } else if (size.width() > 0 && size.height() > 0) {
        // if size non-zero, scale, and record (to recover after zero)
        double sx = (float)size.width() / oldSize.width(),
          sy = (float)size.height() / oldSize.height();
        QGraphicsView::ViewportAnchor anchor = transformationAnchor();
        // FIXME: this scaling isn't perfect, but it's close...
        setTransformationAnchor(QGraphicsView::NoAnchor);
        scale(sx, sy);
        setTransformationAnchor(anchor);
        oldSize = size;
      }
    } else if (_rescaleMode == WidgetGeometry) {
      updateSceneGeometry(scene());
    }
  }
  updateSceneGeometry(overlay());
}

void PlotView::updateSceneGeometry() {
  if (_rescaleMode == WidgetGeometry)
    updateSceneGeometry(scene());
}

void PlotView::updateSceneGeometry(QGraphicsScene *scene) {
  QList<QGraphicsItem *> items = scene->items();
  QRectF geometry = viewport()->rect();
  if (this->scene() == scene) // i.e., not the overlay
    geometry = mapToScene(viewport()->rect()).boundingRect();
  scene->setSceneRect(geometry);
  for (int i = 0; i < items.size(); i++) {
    QGraphicsWidget *widget =
      qgraphicsitem_cast<QGraphicsWidget *>(items[i]);
    if (widget && !items[i]->parentItem())
      widget->setGeometry(geometry);
  }
}

// Needed to know which view is being painted, like when one needs to
// use the GL context of the currently painting QGLWidget, and more
void PlotView::paintEvent(QPaintEvent *event) {
  _isPainting = true;
  QGraphicsView::paintEvent(event);
  _isPainting = false;
}

/* Overlays:
   There is an obvious need for drawing a foreground in viewport
   coordinates. Views often need guides, like axis labels and other
   indicators drawn within the viewport but not in the global
   scene. It is probably possible to make scene items draw relative to
   the currently drawing viewport, but this has some problems:
   1) Tedious to perform the extra mapping steps
   2) The bounding boxes must span the entire scene (inefficient)
   3) Extra work to show/hide per-view (and inefficient)
   4) Not possible to use caching (I think)
   Moreover, it does not make much sense to have scene-level items with
   view-level drawing.
   
   This functionality attempts to formalize and simplify the user of
   overlays, but there is one major drawback: no events. It's
   theoretically possible to hack together some sort of event
   filtering, but usually overlays are non-interactive indicators (at
   least they are in GGobi).
   
   Just add overlays to the overlay scene of a view. Time will
   prove whether this stuff is useful.
 */

void PlotView::drawMetaScene(OverlayScene *scene, QPainter *painter,
                             QRectF rect)
{
  QTransform original = painter->worldTransform();
  painter->setWorldTransform(QTransform()); // we draw in pixels
  rect = viewportTransform().mapRect(rect);
  scene->drawOverlay(painter, rect);
  painter->setWorldTransform(original);
}

// NOTE: foreground is not cached by QGraphicsView, probably OK
void PlotView::drawForeground(QPainter *painter, const QRectF &rect) {
  QGraphicsView::drawForeground(painter, rect);
  //  drawMetaLayers(_overlays, painter, rect);
  drawMetaScene(_overlay, painter, rect);
}

/* Not sure if we need this
void PlotView::drawBackground(QPainter *painter, const QRectF &rect) {
  QGraphicsView::drawBackground(painter, rect);
  drawMetaScene(underlay(), painter, rect);
}
*/

#include <QVarLengthArray>

void OverlayScene::drawOverlay(QPainter *painter, QRectF exposed) {
  QList<QGraphicsItem *> items = this->items();
  QVarLengthArray<QGraphicsItem *, 5> overlayItems(items.size());
  QVarLengthArray<QStyleOptionGraphicsItem, 5> options(items.size());
  QLineF v1(0, 0, 1, 0);
  QLineF v2(0, 0, 0, 1);
  for (int i = 0; i < items.size(); i++) {
    QTransform tform = painter->worldTransform(); // should be identity
    if (items[i]->flags() & QGraphicsItem::ItemIgnoresTransformations)
      tform = items[i]->deviceTransform(tform);
    else tform = items[i]->sceneTransform() * tform;
    options[i].exposedRect = tform.inverted().mapRect(exposed);
#if QT_VERSION < 0x40600
    options[i].rect = items[i]->boundingRect().toRect();
    options[i].matrix = tform.toAffine();
    options[i].levelOfDetail = qSqrt(tform.map(v1).length() *
                                     tform.map(v2).length());
#endif
    overlayItems[i] = items[i];
  }
  drawItems(painter, items.size(), overlayItems.data(), options.data(),
            _view->viewport());
}

// NOTE: only makes sense in single-threaded apps (like R)
PlotView *PlotView::paintingView(QGraphicsScene *scene) {
  QList<QGraphicsView *> views = scene->views();
  PlotView *paintingView = NULL;
  for (int i = 0; i < views.size() && !paintingView; i++) {
    PlotView *view = qobject_cast<PlotView *>(views[i]);
    if (view && view->isPainting())
      paintingView = view;
  }
  return paintingView;
}

void PlotView::setOpenGL(bool opengl) {
  if (opengl) {
#ifdef QT_OPENGL_LIB
    setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#else
    qWarning("OpenGL not supported by this build of Qt and/or qtpaint");
#endif
  } else setViewport(new QWidget);
}
