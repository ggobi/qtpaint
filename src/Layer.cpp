#include <QStyleOptionGraphicsItem>
#include <QGraphicsGridLayout>
#include <QGraphicsSceneEvent>

#include <stdio.h>

#define GL_GLEXT_PROTOTYPES 1
#include "OpenGLPainter.hpp"
#include "ScenePainter.hpp"
#include "Layer.hpp"
#include "PlotView.hpp"

using namespace Qanviz;

static bool fboMultisamplingFailed = false;

static QtMsgHandler prevMsgHandler = NULL;

void fboDebugMsgCatcher(QtMsgType type, const char *msg)
{
  qInstallMsgHandler(prevMsgHandler);
  switch (type) {
  case QtDebugMsg:
    fboMultisamplingFailed = true;
    break;
  case QtWarningMsg:
    qWarning("%s", msg);
    break;
  case QtCriticalMsg:
    qCritical("%s", msg);
    break;
  case QtFatalMsg:
    qFatal("%s", msg);
  }
}

void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
  QtPainter *qvpainter = NULL;
#ifdef QT_OPENGL_LIB
  QPainter *fboPainter;
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

    // NOTE: In Qt 4.6, 'painter' would already target an FBO, if we
    // were using the 'OpenGL2' paint engine. We have decided to stick
    // with the original engine for now, as the OpenGL2 engine relies
    // heavily on shaders, which is slow for our use case.
    
    // Apparently, we must use the QGLContext associated with
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
      // GC during paint callback may have reset this
      if (qglWidget->context() != QGLContext::currentContext())
        qglWidget->makeCurrent();
      // NOTE: need Qt 4.6 for antialiasing to work with FBOs
#if QT_VERSION >= 0x40600
      if (!fboMultisamplingFailed) {
        QGLFramebufferObjectFormat fboFormat;
        fboFormat.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
        fboFormat.setSamples(4); // 4X antialiasing should be enough?
        prevMsgHandler = qInstallMsgHandler(fboDebugMsgCatcher);
        fbo = new QGLFramebufferObject(size, fboFormat);
        qInstallMsgHandler(prevMsgHandler);
        if (fboMultisamplingFailed) {
          delete fbo;
          fbo = NULL;
        }
      }
#endif
      if (!fbo)
        fbo = new QGLFramebufferObject(size);
      // clear the FBO
      fboPainter = new QPainter(fbo);
      fboPainter->setCompositionMode(QPainter::CompositionMode_Source);
      fboPainter->fillRect(0, 0, size.width(), size.height(), Qt::transparent);
      fboPainter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      qvpainter = new OpenGLPainter(fboPainter, context);
      qvpainter->setTransform(painter->worldTransform());
    }
  }
#endif
  
  if (!qvpainter) // fallback to Qt renderer
    qvpainter = new QtPainter(painter);

  paintPlot(qvpainter, option->exposedRect);

  delete qvpainter;
#ifdef QT_OPENGL_LIB
  if (fbo) { // silliness: download to image, only to upload to texture
    painter->setWorldMatrixEnabled(false);
    qglWidget->makeCurrent(); // gc during callback may have cleared this
    // need to tell Qt that 'fboImage' is actually premultiplied
    QImage fboImage = fbo->toImage();
    const uchar *data = fboImage.bits(); // no deep copy
    QImage premultImage = QImage(data,
                                 fboImage.width(),
                                 fboImage.height(),
                                 QImage::Format_ARGB32_Premultiplied);
    // Not sure why this can't be (0, 0)...
    painter->drawImage(QPointF(1, -1), premultImage);
    delete fboPainter;
    delete fbo;
  }
#endif
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
  indexScene->setItemIndexMethod(QGraphicsScene::NoIndex);
  indexScene->clear();
  scenePainter = new ScenePainter(indexScene);
  scenePainter->setIndexMode(true);
  paintPlot(scenePainter, boundingRect());
  indexScene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
}
    
void Layer::invalidateIndex() {
  delete scenePainter;
  scenePainter = NULL;
}

Layer *Layer::layerAt(int row, int col) {
  QGraphicsLayoutItem *layout_item = gridLayout()->itemAt(row, col);
  Layer *layer = NULL;
  if (layout_item && layout_item->graphicsItem()->isWidget()) {
    QGraphicsWidget *widget =
      static_cast<QGraphicsWidget *>(layout_item->graphicsItem());
    layer = qobject_cast<Layer *>(widget);
  }
  return layer;
}

void hideCellAlreadyTakenHandler(QtMsgType type, const char *msg)
{
  qInstallMsgHandler(prevMsgHandler);
  switch (type) {
  case QtDebugMsg:
    qDebug("%s", msg);
    break;
  case QtWarningMsg:
    if (!QRegExp("QGridLayoutEngine::addItem: Cell \\(\\d+, \\d+\\) already taken").exactMatch(msg))
      qWarning("%s", msg);
    break;
  case QtCriticalMsg:
    qCritical("%s", msg);
    break;
  case QtFatalMsg:
    qFatal("%s", msg);
  }
}

void Layer::addLayer(Layer *layer, int row, int col,
                     int rowSpan, int colSpan)
{
  if (!layer) {
    qWarning("Layer::addLayer: cannot add null layer");
    return;
  }
  if (scene() && layer->scene() != scene()) {
    scene()->addItem(layer);
  }
  // FIXME: we hide this message, but it is a serious one: we are
  // using Qt in an unsupported manner.
  prevMsgHandler = qInstallMsgHandler(hideCellAlreadyTakenHandler);
  gridLayout()->addItem(layer, row, col, rowSpan, colSpan);
  qInstallMsgHandler(prevMsgHandler);
  layer->setZValue(childItems().size());
}

// This method in QGraphicsItem is buggy (assumes 'self' ignores transforms)
QTransform Layer::deviceTransform(QGraphicsView *view) const
{
  return sceneTransform() * view->viewportTransform();
}

QTransform Layer::deviceTransform(QGraphicsSceneEvent *event) const
{
  QGraphicsView *view = viewForEvent(event);
  if (!view)
    return sceneTransform();
  return sceneTransform() * view->viewportTransform();
}


QGraphicsView *Layer::viewForEvent(QGraphicsSceneEvent *event) {
  return qobject_cast<QGraphicsView*>(event->widget()->parent());
}

QGraphicsGridLayout *Layer::gridLayout() const {
  return static_cast<QGraphicsGridLayout *>(layout());
}

/* Block events that call update() (usually needlessly) */
void Layer::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
  Q_UNUSED(event);
}
void Layer::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
  Q_UNUSED(event);
}
void Layer::focusInEvent ( QFocusEvent * event ) {
}
void Layer::focusOutEvent ( QFocusEvent * event ) {
}


bool Layer::event(QEvent *event) {
  switch (event->type()) {
    /* do not redraw (update) when activation state changes */
  case QEvent::WindowActivate:
  case QEvent::WindowDeactivate:
    return(true);
  default:
    break;
  }
  return QGraphicsWidget::event(event);
}
