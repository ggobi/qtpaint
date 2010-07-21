#ifndef PAINTER_H
#define PAINTER_H

#include <QRectF>
#include <QImage>
#include <QColor>
#include <QFont>
#include <QFontMetrics>

#include <math.h> // for fabs()

namespace Qanviz {
  class Painter {

  private:
    QTransform savedTransform;
    bool _transformEnabled;
    bool _rasterize;
    double _glyphSize;
    
  public:

    Painter() : _transformEnabled(true), _rasterize(true), _glyphSize(1.0) { }

    virtual ~Painter() { }
    
    // accessors
    virtual QRectF deviceRect() const = 0;

    void setLimits(QRectF limits, bool combined = false) {
      QTransform tform;
      QRectF device = deviceRect();
      tform.scale(device.width() / limits.width(),
                  -device.height() / limits.height());
      tform.translate(-limits.left(), -limits.bottom());
      setTransform(tform, combined);
    }
    virtual void setTransform(const QTransform& tform, bool combined = false) {
      Q_UNUSED(combined);
      // just checks for rotated matrices, which are unsupported
      // (they cannot map text extents to user space, and besides,
      // there are currently no use cases for rotation).
      if (tform.isRotating()) {
        qWarning("Painter::setTransform: rotating matrices not supported");
      }
    }
    virtual const QTransform& transform() const = 0;

    void setTransformEnabled(bool enabled = true) {
      if (enabled == _transformEnabled)
        return;
      _transformEnabled = enabled;
      if (enabled)
        setTransform(savedTransform);
      else {
        QTransform ident;
        savedTransform = transform();
        setTransform(ident);
      }
    }
    bool transformEnabled() { return _transformEnabled; }
    
    // stroke/fill
    virtual bool hasStroke() const = 0;
    virtual bool hasFill() const = 0;
    virtual void setHasStroke(bool enabled) = 0;
    virtual void setHasFill(bool enabled) = 0;
    
    // colors
    virtual void setStrokeColor(QColor color) = 0;
    virtual QColor strokeColor() const = 0;
    virtual void setFillColor(QColor color) = 0;
    virtual QColor fillColor() const = 0;
    
    // font
    virtual void setFont(QFont font) = 0;
    virtual QFont font() const = 0;
    
    // line aesthetics  
    virtual void setLineWidth(uint width) = 0;
    virtual uint lineWidth() const = 0;
    virtual void setDashes(double *dashes, int n) = 0;
    virtual const double * dashes(int *n) const = 0;
    virtual void setLineCap(Qt::PenCapStyle cap) = 0;
    virtual Qt::PenCapStyle lineCap() const = 0;
    virtual void setLineJoin(Qt::PenJoinStyle join) = 0;
    virtual Qt::PenJoinStyle joinStyle() const = 0;
    virtual void setMiterLimit(double limit) = 0;
    virtual double miterLimit() const = 0;

    // antialiasing option
    virtual void setAntialias(bool antialias) = 0;
    virtual bool antialias() const = 0;
    
    // should rasterizing optimizations be employed?
    // good for screen, perhaps undersirable off-screen
    bool rasterize() const {
      return _rasterize;
    }
    virtual void setRasterize(bool rasterize = true) {
      _rasterize = rasterize;
    }

    // glyph size (magnification)
    // now part of the state, like font
    // not likely to be managed by a backend, so we manage it here
    virtual void setGlyphSize(double glyphSize) {
      _glyphSize = glyphSize;
    }
    virtual double glyphSize() {
      return _glyphSize;
    }
    
    // draw lines
    virtual void drawPolyline(double *x, double *y, int n) = 0;
    virtual void drawSegments(double *x0, double *y0, double *x1, double *y1,
                              int n) = 0;

    // draw path
    virtual void drawPath(QPainterPath path) = 0;
    
    // draw points (pixels)
    virtual void drawPoints(double *x, double *y, int n) = 0;
    
    // draw shapes
    // NOTE: if drawing many shapes of same size, use drawGlyphs
    virtual void drawRectangles(double *x, double *y, double *w, double *h,
                                int n) = 0;
    // NOTE: radius is in pixels, as it is not associated with an
    // axis. Circles of fixed size are useful, such as in bubble
    // charts. To make a circle that scales (a rare requirement),
    // approximate with a polygon.
    // FIXME: radius could still be double precision
    virtual void drawCircle(double x, double y, int r) = 0;
    virtual void drawPolygon(double *x, double *y, int n) = 0;
    
    // draw text
    /* 'lines' indicates that each string is a single line, positioned
       at the baseline. Otherwise, text is laid out as possibly
       multi-lined block, positioned by its bounding top-left corner.
    */
    virtual void drawText(const char * const *strs, double *x, double *y,
                          int n, Qt::Alignment flags, double rot) = 0;
    
    // overall font metrics
    // FIXME: is there any way to reverse scale when tform is
    // rotated? (any way to unrotate a matrix?)
    void fontMetrics(float *ascent, float *descent) {
      QFontMetrics metrics = QFontMetrics(font());
      float yscale = fabs(transform().m22());
      *ascent = metrics.ascent() / yscale;
      *descent = metrics.descent() / yscale;
    }
    // necessary for centering/aligning text
    QVector<QRectF> textExtents(const char * const *strs, int n,
                                Qt::Alignment alignment = 0)
    {
      QVector<QRectF> rects(n);
      QFontMetrics metrics = QFontMetrics(font());
      QTransform rtform;
      rtform.scale(fabs(1 / transform().m11()), fabs(1 / transform().m22()));
      for (int i = 0; i < n; i++) {
        QString qstr = QString::fromLocal8Bit(strs[i]);
        if (qstr.count(QChar::LineSeparator))
          rects[i] = metrics.boundingRect(0, 0, 0, 0, alignment, qstr);
        // Apparently this is necessary for good vertical bounds
        // It's noted to be slow on Windows, so might want strWidth method
        else rects[i] = metrics.tightBoundingRect(qstr);
        rects[i] = rtform.mapRect(rects[i]); // map from pixels
      }
      return rects;
    }
    
    // image
    virtual void drawImage(const QImage &image, double x, double y, int sx = 0,
                           int sy = 0, int sw = -1, int sh = -1) = 0;
    
    // drawing glyphs (same path, many places) we take size and color
    // overrides for optimization purposes.
    // the path is generally rasterized, and images are generally slow
    // to pass to the hardware
    virtual void drawGlyphs(const QPainterPath &path, double *x, double *y,
                            double *size, QColor *stroke,
                            QColor *fill, int n) = 0;
    
    virtual void drawGlyphs(const QImage &image, double *x, double *y,
                            int n)
    {
      prepareDrawGlyphs();
      drawSomeGlyphs(image, x, y, n);
      finishDrawGlyphs();
    }

    virtual QImage rasterizeGlyph(const QPainterPath &path) = 0;

  protected:
    virtual void prepareDrawGlyphs(void) { }
    virtual void finishDrawGlyphs(void) { }
    virtual void drawSomeGlyphs(const QImage &image, double *x, double *y,
                                int n)
    {
      for (int i = 0; i < n; i++)
        drawImage(image, x[i], y[i]);
    }

  };
}

#endif
