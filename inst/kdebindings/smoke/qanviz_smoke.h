#ifndef QANVIZ_SMOKE_H
#define QANVIZ_SMOKE_H

#include <smoke.h>

// Defined in smokedata.cpp, initialized by init_qanviz_Smoke(), used by all .cpp files
extern "C" SMOKE_EXPORT Smoke* qanviz_Smoke;
extern "C" SMOKE_EXPORT void init_qanviz_Smoke();
extern "C" SMOKE_EXPORT void delete_qanviz_Smoke();

#ifndef QGLOBALSPACE_CLASS
#define QGLOBALSPACE_CLASS
class QGlobalSpace { };
#endif

#endif
