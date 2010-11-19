#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

#include <qtbase.h>

SEXP
scene_addPoints(SEXP scene, SEXP x, SEXP y, SEXP radius)
{
  QGraphicsScene* s = unwrapSmoke(scene, QGraphicsScene);
  int i, n = length(x);
  for (i = 0; i < n; i++) {
    // QGraphicsEllipseItem *item = s->addEllipse(REAL(x)[i], REAL(y)[i], REAL(radius)[0], REAL(radius)[0]);
    QGraphicsEllipseItem *item =
      s->addEllipse(0.0, 0.0, REAL(radius)[0], REAL(radius)[0]);
    item->setPos(REAL(x)[i], REAL(y)[i]);
    item->setFlags(QGraphicsItem::ItemIsSelectable | 
                   QGraphicsItem::ItemIgnoresTransformations);
  }
  return R_NilValue;
}


SEXP
scene_addLines(SEXP scene, SEXP x, SEXP y, SEXP lwd)
{
  QGraphicsScene* s = unwrapSmoke(scene, QGraphicsScene);
  int nlwd = length(lwd);
  int i, n = length(x);
  for (i = 1; i < n; i++) {
    s->addLine(REAL(x)[i-1], REAL(y)[i-1], 
               REAL(x)[i],   REAL(y)[i], 
               QPen(Qt::black, REAL(lwd)[(i-1) % nlwd]));
  }
  return R_NilValue;
}


SEXP
scene_addSegments(SEXP scene, SEXP x1, SEXP y1, SEXP x2, SEXP y2, SEXP lwd)
{
  QGraphicsScene* s = unwrapSmoke(scene, QGraphicsScene);
  int nlwd = length(lwd);
  int i, n = length(x1);
  for (i = 0; i < n; i++) {
    s->addLine(REAL(x1)[i], REAL(y1)[i], REAL(x2)[i], REAL(y2)[i], 
               QPen(Qt::black, REAL(lwd)[(i-1) % nlwd]));
  }
  return R_NilValue;
}



// QGraphicsPolygonItem * addPolygon ( const QPolygonF & polygon, const QPen & pen = QPen(), const QBrush & brush = QBrush() )


SEXP
scene_addRect(SEXP scene, SEXP x, SEXP y, SEXP w, SEXP h)
{
  QGraphicsScene* s = unwrapSmoke(scene, QGraphicsScene);
  int nw = length(w), nh = length(h);
  int i, n = length(x);
  for (i = 0; i < n; i++) {
    s->addRect(REAL(x)[i], REAL(y)[i], REAL(w)[i % nw], REAL(h)[i % nh]);
  }
  return R_NilValue;
}

SEXP
scene_addText(SEXP scene, SEXP x, SEXP y, SEXP labels, SEXP html)
{
  QGraphicsScene* s = unwrapSmoke(scene, QGraphicsScene);
  int nlab = length(labels);
  int i, n = length(x);
  for (i = 0; i < n; i++) {
    QGraphicsTextItem *ti = s->addText(QString());
    ti->setFont(QFont("Arial"));
    if (LOGICAL(html)[0]) {
      ti->setHtml(QString::fromLocal8Bit(CHAR(asChar(STRING_ELT(labels, i % nlab)))));
      ti->setOpenExternalLinks(true);
      ti->setToolTip("I am HTML!");
    }
    else {
      ti->setPlainText(QString::fromLocal8Bit(CHAR(asChar(STRING_ELT(labels, i % nlab)))));
    }
    ti->setPos(REAL(x)[i], REAL(y)[i]);
    ti->setFlags(QGraphicsItem::ItemIsMovable | 
                 QGraphicsItem::ItemIsSelectable | 
                 QGraphicsItem::ItemIsFocusable | 
                 QGraphicsItem::ItemIgnoresTransformations);
  }
  return R_NilValue;
}

SEXP qt_qsetItemFlags(SEXP x, SEXP flag, SEXP status)
{
  QList<QGraphicsItem*> ilist = unwrapSmoke(x, QGraphicsScene)->items();
  bool bstatus = (bool) asLogical(status);
  QGraphicsItem::GraphicsItemFlag bflag =
    (QGraphicsItem::GraphicsItemFlag) asInteger(flag);
  for (int i = 0; i < ilist.size(); ++i) {
      ilist[i]->setFlag(bflag, bstatus);
  }
  return R_NilValue;
}


SEXP qt_qsetTextItemInteraction(SEXP x, SEXP mode)
{
  QList<QGraphicsItem*> ilist = unwrapSmoke(x, QGraphicsScene)->items();
  QGraphicsTextItem *textitem;
  QString smode = sexp2qstring(mode);
  if (smode == "none") {
    for (int i = 0; i < ilist.size(); ++i) {
      textitem = qgraphicsitem_cast<QGraphicsTextItem *>(ilist[i]);
      if (textitem)
        textitem->setTextInteractionFlags(Qt::NoTextInteraction);
    }
  }
  else if (smode == "editor") {
    for (int i = 0; i < ilist.size(); ++i) {
      textitem = qgraphicsitem_cast<QGraphicsTextItem *>(ilist[i]);
      if (textitem)
        textitem->setTextInteractionFlags(Qt::TextEditorInteraction);
    }
  }
  else if (smode == "browser") {
    for (int i = 0; i < ilist.size(); ++i) {
      textitem = qgraphicsitem_cast<QGraphicsTextItem *>(ilist[i]);
      if (textitem)
        textitem->setTextInteractionFlags(Qt::TextBrowserInteraction);
    }
  }
  return R_NilValue;
}
