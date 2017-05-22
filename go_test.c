#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "icy_mem.h"
#include "icy_table.h"
#include "mem.h"
#include "int_set.h"
#include "int_set.c"
#include "log.h"
int main(){
  logd("Hellos\n");
  int_set * table = int_set_create("my-ints");
  int_set_set(table, 5);
  int_set_set(table, 10);
  int_set_print(table);
  return -1;
}
