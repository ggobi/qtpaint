#include <qtbase.h>
#include <R_ext/Rdynload.h>

#define CALLDEF(name, n)  {#name, (DL_FUNC) &name, n}

static R_CallMethodDef CallEntries[] = {
  {NULL, NULL, 0}
};

#include <qanviz_smoke.h>

void init_smoke(void) {
  init_qanviz_Smoke();
  qanviz_Smoke = registerSmokeModule(qanviz_Smoke);
}

extern "C" {
  void R_init_qtpaint(DllInfo *dll)
  {
    init_smoke();
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  }
}
