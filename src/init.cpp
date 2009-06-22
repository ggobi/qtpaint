#include <R.h>
#include <R_ext/Rdynload.h>

#define CALLDEF(name, n)  {#name, (DL_FUNC) &name, n}

static R_CallMethodDef CallEntries[] = {
  {NULL, NULL, 0}
};

extern "C" {
  void R_init_qtpaint(DllInfo *dll)
  {
    // TODO: add calldefs
  }
}
