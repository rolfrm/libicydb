#define ICY_HIDDEN __attribute__((visibility("hidden")))
#include "icydb.h"

#ifdef USE_VALGRIND
#include <valgrind/memcheck.h>
#define MAKE_UNDEFINED(x) VALGRIND_MAKE_MEM_UNDEFINED(&(x),sizeof(x));
#define MAKE_NOACCESS(x) VALGRIND_MAKE_MEM_NOACCESS(&(x),sizeof(x));
#else
#define MAKE_UNDEFINED(x) UNUSED(x);
#define MAKE_NOACCESS(x) UNUSED(x);
#endif
