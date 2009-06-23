#include <QGraphicsScene>
#include <QGraphicsWidget>

#include "conversion.h"

extern "C" {
  SEXP qt_qscene(void)
  {
    QGraphicsScene* scene = new QGraphicsScene(NULL);
    return wrapQGraphicsScene(scene);
  }
  
  SEXP qt_qaddWidget_QGraphicsScene(SEXP rself, SEXP ritem) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    QGraphicsWidget *item = unwrapQObject(ritem, QGraphicsWidget);
    scene->addItem(item);
    // Make sure that item is not deleted before its scene
    // FIXME: leaky -- needs to happen in itemChange() upon ItemSceneChange,
    // but that requires always using a QGraphicsWidget subclass
    addQGraphicsWidgetReference(item, scene);
    return rself;
  }
  
  SEXP qt_qsetBackground_QGraphicsScene(SEXP rself, SEXP rcolor) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    scene->setBackgroundBrush(QBrush(asQColor(rcolor)));
    return rself;
  }
  
  SEXP qt_qupdate_QGraphicsScene(SEXP rself) {
    QGraphicsScene *scene = unwrapQObject(rself, QGraphicsScene);
    QList<QGraphicsItem *> items = scene->items();
    for (int i = 0; i < items.size(); i++) {
      QGraphicsItem *item = items[i];
      QGraphicsItem::CacheMode mode = item->cacheMode();
      item->setCacheMode(QGraphicsItem::NoCache);
      item->setCacheMode(mode);
    }
    scene->update();
    return rself;
  }
}
