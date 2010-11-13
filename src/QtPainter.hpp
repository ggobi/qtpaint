#ifndef QTPAINTER_H
#define QTPAINTER_H

#include <QPainter>
#include "QtBasePainter.hpp"

namespace Qanviz {
  class QtPainter : public QtBasePainter {
    
  private:
    QPaintDevice *_device;
    bool ownPainter;
    QImage *glyphBuffer;
    void initialize() {
      setAntialias(true);
    }
    
  protected:
    QPainter *painter;
    
  public:
    
    QtPainter(QPaintDevice *device) :
      _device(device), ownPainter(true), glyphBuffer(NULL),
      painter(new QPainter(device))
    {
      initialize();
    }

    QtPainter(QPainter *painter) :
      _device(painter->device()), ownPainter(false), painter(painter)
    {
      initialize();
    }

    virtual ~QtPainter() {
      if (ownPainter) {
        delete painter;
      }
    }

    QPaintDevice *device() const { return _device; }

    QRectF deviceRect() const {
      return QRectF(0, 0, device()->width(), device()->height());
    }

    // opacity
    qreal opacity() { return painter->opacity(); }
    void setOpacity(qreal opacity) { painter->setOpacity(opacity); }

    // scaling
    void setTransform(const QTransform& tform, bool combined = false) {
      painter->setWorldTransform(tform, combined);
      // force update
      painter->save();
      painter->restore();
    }

    const QTransform& transform() const {
      return painter->worldTransform();
    }
    
    // font
    void setFont(QFont font);
    QFont font() const {
      return painter->font();
    }
    
    // antialiasing
    void setAntialias(bool antialias) {
      painter->setRenderHint(QPainter::Antialiasing, antialias);
      //painter->setRenderHint(QPainter::HighQualityAntialiasing, antialias);
    }
    bool antialias() const {
      return painter->renderHints() & QPainter::Antialiasing;
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
        
  protected:
    virtual void setBrush(QBrush brush) {
      painter->setBrush(brush);
    }
    virtual QBrush brush() const { return painter->brush(); }
    virtual void setPen(QPen pen) {
      painter->setPen(pen);
    }
    virtual QPen pen() const { return painter->pen(); }
    
    virtual void prepareDrawGlyphs(void);
    virtual void finishDrawGlyphs(void);
    virtual void drawSomeGlyphs(const QImage &image, double *x, double *y,
                                int n);  
  };
}

#endif
