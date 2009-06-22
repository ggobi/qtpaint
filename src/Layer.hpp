#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsWidget>
#include <QGraphicsGridLayout>
#include <QGLContext>
//#include <QGraphicsView>
#include "Painter.hpp"
#include "ScenePainter.hpp"

namespace QViz {
  class Layer : public QGraphicsWidget {
    Q_OBJECT
    
    private:
    
    QRectF _limits;
    QGraphicsScene *indexScene;
    ScenePainter *scenePainter;
    
    // QGraphicsWidget assumes a (0, 0, width, height) bounding
    // rectangle, where width and height are from the geometry.
    // We want to draw into data space, so we allow the
    // user to specify the data limits, and then synchronize the item
    // transform so that everything ends up in the geometry specified
    // by the layout.
    
    void updatePlotMatrix() {
      QTransform plotMatrix;
      if (!_limits.isNull()) {
        QRectF bounds = rect();
        plotMatrix.scale(bounds.width() / _limits.width(),
                         -bounds.height() / _limits.height());
        plotMatrix.translate(-_limits.left(), -_limits.bottom());
      }
      setTransform(plotMatrix);
    }

    QVector<int> itemIndices(QList<QGraphicsItem *> items) {
      QVector<int> inds(items.size());
      for (int i = 0; i < items.size(); i++)
        inds[i] = scenePainter->itemIndex(items[i]);
      return inds;
    }
    
    public:
    
    Layer() : indexScene(new QGraphicsScene()), scenePainter(NULL) {
      QGraphicsGridLayout *layout = new QGraphicsGridLayout;
      layout->setContentsMargins(0, 0, 0, 0);
      setLayout(layout);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding,
                    QSizePolicy::DefaultType);
      setFlags(QGraphicsItem::ItemClipsToShape |
               QGraphicsItem::ItemClipsChildrenToShape);
    }
    ~Layer() {
      delete indexScene;
      if (scenePainter)
        delete scenePainter;
    }

    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    virtual void paintPlot(Painter *painter, QRectF exposed) = 0;

    void setGeometry(const QRectF &rect) {
      QGraphicsWidget::setGeometry(rect);    
      // update transform to point current limits to new geometry
      updatePlotMatrix();
    }

    QRectF boundingRect() const {
      if (!_limits.isNull()) {
        return _limits;
      } else return QGraphicsWidget::boundingRect();
    }

    QPainterPath shape() const {
      if (!_limits.isNull()) {
        QPainterPath path;
        path.addRect(_limits);
        return path;
      } else return QGraphicsWidget::shape();
    }
    
    
    virtual void setLimits(QRectF limits) {
      if (limits != _limits) {
        prepareGeometryChange();
        _limits = limits;
        // update transform to point new limits to current geometry
        updatePlotMatrix();
      }
    }
    void setLimits(qreal x0, qreal y0, qreal x1, qreal y1) {
      setLimits(QRectF(x0, y0, x1 - x0, y1 - y0));
    }
    QRectF limits() const {
      return _limits;
    }

    void setItemIndexMethod(QGraphicsScene::ItemIndexMethod method) {
      indexScene->setItemIndexMethod(method);
    }
    QGraphicsScene::ItemIndexMethod itemIndexMethod() {
      return indexScene->itemIndexMethod();
    }
    
    void ensureIndex() {
      if (scenePainter)
        return;
      indexScene->clear();
      scenePainter = new ScenePainter(indexScene);
      scenePainter->setIndexMode(true);
      paintPlot(scenePainter, boundingRect());
    }
    
    void invalidateIndex() {
      delete scenePainter;
      scenePainter = NULL;
    }
    
    QVector<int> items(const QPointF & pos)
    {
      ensureIndex();
      return itemIndices(indexScene->items(pos));
    }
    
    QVector<int> items(const QRectF & rectangle,
                       Qt::ItemSelectionMode mode = Qt::IntersectsItemShape)
    {
      ensureIndex();
      return itemIndices(indexScene->items(rectangle, mode));
    }

    
    QVector<int> items(const QPolygonF & polygon,
                       Qt::ItemSelectionMode mode = Qt::IntersectsItemShape)
    {
      ensureIndex();
      return itemIndices(indexScene->items(polygon, mode));
    }
    
    QVector<int> items(const QPainterPath & path,
                       Qt::ItemSelectionMode mode = Qt::IntersectsItemShape)
    {
      ensureIndex();
      return itemIndices(indexScene->items(path, mode));
    }
    
    QVector<int>
    collidingItems(const QGraphicsItem *item,
                   Qt::ItemSelectionMode mode = Qt::IntersectsItemShape)
    {
      ensureIndex();
      return itemIndices(indexScene->collidingItems(item, mode));
    }
  };
}

#endif
