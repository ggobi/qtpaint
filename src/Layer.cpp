#include <QStyleOptionGraphicsItem>

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
      fbo = new QGLFramebufferObject(size);
      // clear the FBO, necessary on at least some Macs
      printf("clearing FBO!\n");
      fboPainter = new QPainter(fbo);
      fboPainter->setCompositionMode(QPainter::CompositionMode_Source);
      fboPainter->fillRect(0, 0, size.width(), size.height(),
                           QColor(0, 255, 0, 255));
      fboPainter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      qvpainter = new OpenGLPainter(fboPainter, context);
      qvpainter->setMatrix(painter->worldMatrix());
    }
  }
  
  if (!qvpainter) // fallback to Qt renderer
    qvpainter = new QtPainter(painter);
  
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
