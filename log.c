#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <errno.h>
#include <unistd.h>

void (* iron_log_printer)(const char * fnt, va_list lst) = NULL;

static void do_log_print(const char * fmt, va_list lst){
  vprintf (fmt, lst);
}
#include <stdbool.h>
void log_print(const char * fmt, ...){
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

int str_index_of_last(const char * str, char symbol){
  int idx = -1;
  
  for(int i = 0; str[i] != 0; i++){
    if(str[i] == symbol)
      idx = i;
  }
  return idx;
}

void iron_log_stacktrace(void)
{
  static const char start[] = "BACKTRACE ------------\n";
  static const char end[] = "----------------------\n";
  
  void *bt[1024];
  int bt_size;
  char **bt_syms;
  int i;
  
  bt_size = backtrace(bt, 1024);
  bt_syms = backtrace_symbols(bt, bt_size);
  printf(start);
  for (i = 1; i < bt_size; i++) {

    //char syscom[256];
    int itemidx = str_index_of_last(bt_syms[i], '(');
    char filename[itemidx + 1];
    strncpy(filename, bt_syms[i],itemidx);
    filename[itemidx] = 0;

    printf("#%d (%s) %s\n", i, filename, bt_syms[i]);
    //sprintf(syscom,"addr2line -j text  -e %s %p", filename, bt[i]); //last parameter is the name of this app
    //system(syscom);
  }
  printf(end);
  free(bt_syms);
}

__thread int logd_enable = 1;
