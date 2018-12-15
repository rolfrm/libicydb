#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
//#include <execinfo.h>
#include <errno.h>
#include <unistd.h>
#include "icydb_int.h"
static void (* iron_log_printer)(const char * fnt, va_list lst) = NULL;

static void do_log_print(const char * fmt, va_list lst){
  vprintf (fmt, lst);
}
#include <stdbool.h>
ICY_HIDDEN void log_print(const char * fmt, ...){
  static bool is_printing = false;
  if(is_printing) return;
  is_printing = true;
  if(iron_log_printer == NULL)
    iron_log_printer = do_log_print;
  va_list args;
  va_start (args, fmt);
  iron_log_printer(fmt, args);
  va_end (args);
  is_printing = false;
}

ICY_HIDDEN void iron_log_stacktrace(void)
{
  // blep
}
