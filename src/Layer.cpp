#include <QStyleOptionGraphicsItem>
#include <QGraphicsGridLayout>
#include <QGraphicsSceneEvent>

#include "OpenGLPainter.hpp"
#include "ScenePainter.hpp"
#include "Layer.hpp"
#include "PlotView.hpp"

using namespace Qanviz;

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
      // NOTE: need Qt 4.6 for antialiasing to work with FBOs
#if QT_VERSION >= 0x40600
      QGLFramebufferObjectFormat fboFormat;
      fboFormat.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
      fboFormat.setSamples(4); // 4X antialiasing should be enough?
      fbo = new QGLFramebufferObject(size, fboFormat);
#else
      fbo = new QGLFramebufferObject(size);
#endif
      // clear the FBO, necessary on at least some Macs
      fboPainter = new QPainter(fbo);
      fboPainter->setCompositionMode(QPainter::CompositionMode_Source);
      fboPainter->fillRect(0, 0, size.width(), size.height(),
                           QColor(0, 0, 0, 0));
      fboPainter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      qvpainter = new OpenGLPainter(fboPainter, context);
      qvpainter->setTransform(painter->worldTransform());
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

void Layer::updatePlotTransform() {
  QTransform plotTransform;
  QRectF bounds = rect();
  if (!_limits.isNull()) {
    plotTransform.scale(bounds.width() / _limits.width(),
                     -bounds.height() / _limits.height());
    plotTransform.translate(-_limits.left(), -_limits.bottom());
  }
  setTransform(plotTransform);
}

QVector<int> Layer::itemIndices(QList<QGraphicsItem *> items) {
  QVector<int> inds(items.size());
  for (int i = 0; i < items.size(); i++)
    inds[i] = scenePainter->itemIndex(items[i]);
  return inds;
}
    
Layer::Layer(QGraphicsItem *parent)
  : QGraphicsWidget(parent), indexScene(new QGraphicsScene()),
    scenePainter(NULL)
{
  QGraphicsGridLayout *layout = new QGraphicsGridLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0.0);
  setLayout(layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding,
                QSizePolicy::DefaultType);
  setCacheMode(QGraphicsItem::DeviceCoordinateCache);
  setMinimumSize(QSizeF(1, 1)); // so layout works
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

void Layer::addLayer(Layer *layer, int row = 0, int col = 0,
                     int rowSpan = 1, int colSpan = 1)
{
  gridLayout()->addItem(layer, row, col, rowSpan, colSpan);
  layer->setZValue(childItems().size());
}

// This method in QGraphicsItem is buggy (assumes 'self' ignores transforms)
QTransform Layer::deviceTransform(QGraphicsView *view) const
{
  if (!view) {
    view = PlotView::paintingView(scene());
    if (!view)
      return sceneTransform();
  }
  return sceneTransform() * view->viewportTransform();
}

QGraphicsView *Layer::viewForEvent(QGraphicsSceneEvent *event) {
  return qobject_cast<QGraphicsView*>(event->widget()->parent());
}

QGraphicsGridLayout *Layer::gridLayout() const {
  return static_cast<QGraphicsGridLayout *>(layout());
}
