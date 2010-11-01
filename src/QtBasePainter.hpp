#ifndef QTBASEPAINTER_H
#define QTBASEPAINTER_H

#include "Painter.hpp"
#include <QPen>
#include <QBrush>

namespace Qanviz {

  /* For targeting any drawing API that stores state in QPen/QBrush */
  
  class QtBasePainter : public Painter {

  private:
    QVector<qreal> dashPattern;
    
  public:
    
    // stroke/fill
    bool hasStroke() const {
      return pen().style() != Qt::NoPen;
    }
    void setHasStroke(bool enabled);
    bool hasFill() const {
      return brush().style() != Qt::NoBrush;
    }
    void setHasFill(bool enabled);
    
    // colors
    QColor strokeColor() const {
      return pen().color();
    }
    void setStrokeColor(QColor color);
    QColor fillColor() const {
      return brush().color();
    }
    void setFillColor(QColor color);

    // opacity (just a getter for now)
    virtual qreal opacity() { return 1.0; }

    // line aesthetics  
    void setLineWidth(uint width);
    uint lineWidth() const {
      return pen().width();
    }
    void setDashes(double *dashes, int n);
    const double * dashes(int *n) const {
      *n = dashPattern.size();
      return dashPattern.data();
    }
    void setLineCap(Qt::PenCapStyle cap);
    Qt::PenCapStyle lineCap() const {
      return pen().capStyle();
    }
    void setLineJoin(Qt::PenJoinStyle join);
    Qt::PenJoinStyle joinStyle() const {
      return pen().joinStyle();
    }
    void setMiterLimit(double limit);
    double miterLimit() const {
      return pen().miterLimit();
    }

    virtual void drawGlyphs(const QPainterPath &path, double *x, double *y,
                            double *size, QColor *stroke, QColor *fill, int n);

    virtual void drawGlyphs(const QImage &image, double *x, double *y, int n)
    {
      Painter::drawGlyphs(image, x, y, n);
    }

    virtual QImage rasterizeGlyph(const QPainterPath &path);
    
  protected:
    bool simplePen() {
      return pen().isCosmetic() && dashPattern.isEmpty();
    }
    virtual void setBrush(QBrush brush) = 0;
    virtual QBrush brush() const = 0;
    virtual void setPen(QPen pen) = 0;
    virtual QPen pen() const = 0;

  private:
    void bufferGlyphs(QPainter &buffer, const QPainterPath &path,
                      double *x, double *y, int n);
  };
}
#endif
