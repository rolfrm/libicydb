#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "log.h"
#include "utils.h"


void _error(const char * file, int line, const char * msg, ...){
  UNUSED(file);UNUSED(line);UNUSED(msg);
  char buffer[1000];  
  va_list arglist;
  va_start (arglist, msg);
  vsprintf(buffer,msg,arglist);
  va_end(arglist);
  loge("%s\n", buffer);
  loge("Got error at %s line %i\n", file,line);
  iron_log_stacktrace();
  raise(SIGINT);
  exit(10);
  raise(SIGINT);
}
