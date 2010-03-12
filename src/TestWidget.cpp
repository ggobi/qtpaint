#include "TestWidget.hpp"
#include "QtPainter.hpp"
#include "OpenGLPainter.hpp"
#include <sys/time.h>

using namespace Qanviz;

#define BEGIN_TIMING gettimeofday(&before, NULL)
#define END_TIMING   gettimeofday(&after, NULL);                        \
  Rprintf("op: %d ogl: %d aa: %d time: %f\n", op, opengl,               \
          painter->antialias(),                                         \
          after.tv_sec + (float)after.tv_usec/1000000 -                 \
          (before.tv_sec + (float)before.tv_usec/1000000))

static void draw(Painter *painter, Operation op, bool opengl) {
  int N = 300000;
  struct timeval before, after;
  painter->setLimits(QRectF(0, 0, 100, 100));
  painter->setHasFill(false);

  switch(op) {
  case Glyph: {
    QPainterPath path;
    path.addEllipse(45, 45, 10, 10);
    //uint circle = painter->registerPath(path);
    double *x = new double[N];
    double *y = new double[N];
    for (int i = 0; i < N; i++) {
      x[i] = 50;//rand()*100.0/RAND_MAX;
      y[i] = 50;//rand()*100.0/RAND_MAX;
    }
  
    BEGIN_TIMING;
    painter->drawGlyphs(path, x, y, NULL, NULL, NULL, N);
    END_TIMING;
    /*
      BEGIN_TIMING
      for (int i = 0; i < 10; i++)
      painter->drawGlyphs(path, x+i, y+i, 1);
      END_TIMING
    */
  }
    break;
  case Polygon: {
    double x[3], y[3];
    x[0] = 45; y[0] = 55;
    x[1] = 55; y[1] = 55;
    x[2] = 50; y[2] = 45;
    BEGIN_TIMING;
    for (int i = 0; i < N; i++)
      painter->drawPolygon(x, y, 3);
    END_TIMING;
  }
    break;
  case Rectangle: {  
    double *x = new double[N];
    double *y = new double[N];
    double *w = new double[N];
    double *h = new double[N];
    for (int i = 0; i < N; i++) {
      x[i] = 45;
      y[i] = 45;
      w[i] = 10;
      h[i] = 10;
    }
    BEGIN_TIMING;
    painter->drawRectangles(x, y, w, h, N);
    END_TIMING;
  }
    break;
  case Point: {
    double *x = new double[N];
    double *y = new double[N];
    for (int i = 0; i < N; i++) {
      x[i] = 50;
      y[i] = 50;
    }
    BEGIN_TIMING;
    painter->drawPoints(x, y, N);
    END_TIMING;
  }
    break;
  case Line: {
    int L = 10000;
    double *x = new double[N/L];
    double *y = new double[N/L];
    for (int i = 0; i < N/L; i++) {
      x[i] = i*3;
      y[i] = 100 * (i % 2);
      //Rprintf("%f %f\n", x[i], y[i]);
    }
  
    BEGIN_TIMING;
    for (int i = 0; i < L; i++)
      painter->drawPolyline(x, y, N / L);
    END_TIMING;
  }
    break;
  case Segment: {
    double *x0 = new double[N];
    double *y0 = new double[N];
    double *x1 = new double[N];
    double *y1 = new double[N];
    for (int i = 0; i < N; i++) {
      x0[i] = 0;
      y0[i] = 0;
      x1[i] = 100;
      y1[i] = 100;
    }
    
    BEGIN_TIMING;
    painter->drawSegments(x0, y0, x1, y1, N);  
    END_TIMING;
  }
    break;
  case Circle:
    BEGIN_TIMING;
    for (int i = 0; i < N; i++)
      painter->drawCircle(50, 50, 5);
    END_TIMING;
    break;
  default:
    error("Unknown drawing operation");
  }
}

void TestWidget::paintEvent(QPaintEvent *event) {
  // put test code here
  QtPainter painter(this); // or OpenGLPainter
  painter.setAntialias(antialias);
  draw(&painter, operation, false);
}

#ifdef QT_OPENGL_LIB
void TestGLWidget::paintEvent(QPaintEvent *event) {
  // put test code here
  OpenGLPainter painter(this);
  draw(&painter, operation, true);
}
#endif

extern "C" {
  SEXP qanviz_painter_test(SEXP r_op, SEXP r_opengl, SEXP r_antialias) {
    QWidget *test = NULL;
    bool antialias = asLogical(r_antialias);
    Operation op = (Operation)asInteger(r_op);
    if (asLogical(r_opengl)) {
#ifdef QT_OPENGL_LIB
      test = new TestGLWidget(op, antialias);
#else
      warning("No OpenGL support, skipping OpenGL benchmarks");
      return R_NilValue;
#endif
    }
    else test = new TestWidget(op, antialias);
    test->show();
    return R_NilValue;
  }
}
