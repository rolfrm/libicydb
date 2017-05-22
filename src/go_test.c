#include "int_set.h"
#include "int_set.c"
#include "log.h"

int main(){
  logd("Hellos\n");
  int_set * table = int_set_create("my-ints");
  int_set_clear(table);
  int_set_set(table, 5);
  int_set_set(table, 10);
  int_set_set(table, -10);
  int_set_set(table, -110);
  int_set_set(table, -11000);
  int_set_set(table, 11030);
  int_set_set(table, 11035);
  
  int_set_print(table);
  return 0;
}
