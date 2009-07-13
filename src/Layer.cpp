#include <QStyleOptionGraphicsItem>
#include <QGraphicsGridLayout>

#include "OpenGLPainter.hpp"
#include "Layer.hpp"
#include "PlotView.hpp"

using namespace QViz;

void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
  QPainter *fboPainter;
  QtPainter *qvpainter = NULL;
  QGLFramebufferObject *fbo = NULL;
  QGLWidget *qglWidget = qobject_cast<QGLWidget *>(widget);
  if (qglWidget) { // on-screen OpenGL
    QGLContext *context = const_cast<QGLContext *>(qglWidget->context());
    qvpainter = new OpenGLPainter(painter, context);
  } else if (cacheMode() != QGraphicsItem::NoCache &&
             QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
    // caching, try FBO
    // if we have direct rendering and FBO support, make use of
    // FBO, but this could still just be in software
    // FIXME: perhaps have a method for setting a hint?
    // Would be nice if Qt did this itself, should profile...
    
    // FIXME: apparently, we must use the QGLContext associated with
    // the view being painted. Thus, PlotView tracks whether it is
    // inside a paintEvent, so we can get the current QGLWidget.
    OverlayScene *overlayScene = qobject_cast<OverlayScene *>(scene());
    if (overlayScene)
      qglWidget = qobject_cast<QGLWidget *>(overlayScene->view()->viewport());
    else {
      QList<QGraphicsView *> views = scene()->views();
      for (int i = 0; i < views.size() && !qglWidget; i++) {
        PlotView *view = qobject_cast<PlotView *>(views[i]);
        if (view && view->isPainting())
          qglWidget = qobject_cast<QGLWidget *>(view->viewport());
      }
    }
    if (qglWidget) {
      QSize size(painter->device()->width(), painter->device()->height());
      QGLContext *context = const_cast<QGLContext *>(qglWidget->context());
      qglWidget->makeCurrent();
      // NOTE: antialiasing is not supported with FBOs, until Qt 4.6
      fbo = new QGLFramebufferObject(size);
      // clear the FBO, necessary on at least some Macs
      fboPainter = new QPainter(fbo);
      fboPainter->setCompositionMode(QPainter::CompositionMode_Source);
      fboPainter->fillRect(0, 0, size.width(), size.height(),
                           QColor(0, 0, 0, 0));
      fboPainter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      qvpainter = new OpenGLPainter(fboPainter, context);
      qvpainter->setMatrix(painter->worldMatrix());
    }
  }
  
  if (!qvpainter) // fallback to Qt renderer
    qvpainter = new QtPainter(painter);

  // NOTE: in QT 4.6 exposedRect will just be the bounding rect, by default
  paintPlot(qvpainter, option->exposedRect);

  delete qvpainter;
  if (fbo) { // silliness: download to image, only to upload to texture
    painter->setWorldMatrixEnabled(false);
    // Not sure why this can't be (0, 0)...
    painter->drawImage(QPointF(1, -1), fbo->toImage());
    delete fboPainter;
    delete fbo;
  }
}

// QGraphicsWidget assumes a (0, 0, width, height) bounding
// rectangle, where width and height are from the geometry.
// We want to draw into data space, so we allow the
// user to specify the data limits, and then synchronize the item
// transform so that everything ends up in the geometry specified
// by the layout.

void Layer::updatePlotMatrix() {
  QMatrix plotMatrix;
  QRectF bounds = rect();
  if (!_limits.isNull()) {
    plotMatrix.scale(bounds.width() / _limits.width(),
                     -bounds.height() / _limits.height());
    plotMatrix.translate(-_limits.left(), -_limits.bottom());
  }
  setMatrix(plotMatrix);
}

QVector<int> Layer::itemIndices(QList<QGraphicsItem *> items) {
  QVector<int> inds(items.size());
  for (int i = 0; i < items.size(); i++)
    inds[i] = scenePainter->itemIndex(items[i]);
  return inds;
}
    
Layer::Layer() : indexScene(new QGraphicsScene()), scenePainter(NULL) {
  QGraphicsGridLayout *layout = new QGraphicsGridLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0.0);
  setLayout(layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding,
                QSizePolicy::DefaultType);
  setFlags(QGraphicsItem::ItemClipsToShape); 
  // WAS: setFlags(QGraphicsItem::ItemClipsToShape, QGraphicsItem::ItemClipsChildrenToShape);
  // I think this is the right choice (good for panels, strips); actually, we
  // want to be able to disable this too (for axes etc)
}
Layer::~Layer() {
  delete indexScene;
  if (scenePainter)
    delete scenePainter;
}
    
void Layer::ensureIndex() {
  if (scenePainter)
    return;
  indexScene->clear();
  scenePainter = new ScenePainter(indexScene);
  scenePainter->setIndexMode(true);
  paintPlot(scenePainter, boundingRect());
}
    
void Layer::invalidateIndex() {
  delete scenePainter;
  scenePainter = NULL;
}
