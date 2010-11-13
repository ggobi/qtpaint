#include "QtPainter.hpp"
#include <QHash>
#include <QGraphicsScene>

namespace Qanviz {
  class ScenePainter : public QtBasePainter {

  private:
    
    QGraphicsScene *_scene;

    bool _indexMode;

    QHash<QGraphicsItem *, int> itemToIndex;
    int itemCount;

    QPen _pen;
    QBrush _brush;
    QTransform _tform;
    
  public:
    
    ScenePainter(QGraphicsScene *scene) : _scene(scene), itemCount(0) { }
    
    QGraphicsScene *scene() { return _scene; }

    int itemIndex(QGraphicsItem *item);

    void setIndexMode(bool enabled = true) {
      if (enabled) {
        setLineWidth(0);
        setDashes(NULL, 0);
      }
      _indexMode = enabled;
    }
    bool indexMode() { return _indexMode; }
    
    QRectF deviceRect() const {
      return _scene->sceneRect();
    }

    // tform
    void setTransform(const QTransform& tform, bool combined = false) {
      if (combined)
        _tform = tform * _tform;
      else _tform = tform;
    }
    const QTransform& transform() const {
      return _tform;
    }
      
    // font
    void setFont(QFont font) {
      _scene->setFont(font);
    }
    QFont font() const {
      return _scene->font();
    }

    // antialiasing option
    // NOTE: antialiasing is a rendering hint, but we are not really
    // rendering here, so this is ignored.
    void setAntialias(bool antialias) {
      Q_UNUSED(antialias);
    }
    bool antialias() const { return false; }

    void setLineWidth(uint width) {
      if (width > 1)
        setIndexMode(false);
      QtBasePainter::setLineWidth(width);
    }
    void setDashes(double *dashes, int n) {
      if (!n)
        setIndexMode(false);
      QtBasePainter::setDashes(dashes, n);
    }
      
    // draw lines
    void drawPolyline(double *x, double *y, int n);
    void drawSegments(double *x0, double *y0, double *x1, double *y1,
                      int n);

    // draw path
    void drawPath(QPainterPath path);

    // draw points (pixels)
    void drawPoints(double *x, double *y, int n);
    
    // draw shapes
    void drawRectangles(double *x, double *y, double *w, double *h, int n);
    // if drawing many circles of same size, use drawGlyphs
    void drawCircle(double x, double y, int r);
    void drawPolygon(double *x, double *y, int n);
    
    // draw text
    void drawText(const char * const *strs, double *x, double *y, int n,
                  Qt::Alignment flags = 0, double rot = 0, double hcex = 1.0,
                  double vcex = 1.0);
    
    // image
    void drawImage(const QImage &image, double x, double y, int sx = 0,
                   int sy = 0, int sw = -1, int sh = -1);
    
    // drawing glyphs (same path, many places)
    void drawGlyphs(const QPainterPath &path, double *x, double *y,
                    double *size, QColor *stroke, QColor *fill, int n);
    void drawGlyphs(const QImage &image, double *x, double *y, int n) {
      QtBasePainter::drawGlyphs(image, x, y, n);
    }
    
  protected:
    virtual void setBrush(QBrush brush) {
      _brush = brush;
    }
    virtual QBrush brush() const { return _brush; }
    virtual void setPen(QPen pen) {
      _pen = pen;
    }
    virtual QPen pen() const { return _pen; }
    
  };
}

