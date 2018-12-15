// This file is auto generated by icy-table.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif


int_set * int_set_create(const char * optional_name){
  static const char * const column_names[] = {(char *)"key"};
  static const char * const column_types[] = {"int"};
  int_set * instance = calloc(sizeof(int_set), 1);
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  icy_table_init((icy_table * )instance, optional_name, 1, (unsigned int[]){sizeof(int)}, (char *[]){(char *)"key"});
  
  return instance;
}

void int_set_insert(int_set * table, int * key, size_t count){
  void * array[] = {(void* )key};
  icy_table_inserts((icy_table *) table, array, count);
}

void int_set_set(int_set * table, int key){
  void * array[] = {(void* )&key};
  icy_table_inserts((icy_table *) table, array, 1);
}

void int_set_lookup(int_set * table, int * keys, size_t * out_indexes, size_t count){
  icy_table_finds((icy_table *) table, keys, out_indexes, count);
}

void int_set_remove(int_set * table, int * keys, size_t key_count){
  size_t indexes[key_count];
  size_t index = 0;
  size_t cnt = 0;
  while(0 < (cnt = icy_table_iter((icy_table *) table, keys, key_count, NULL, indexes, array_count(indexes), &index))){
    icy_table_remove_indexes((icy_table *) table, indexes, cnt);
    index = 0;
  }
}

void int_set_clear(int_set * table){
  icy_table_clear((icy_table *) table);
}

void int_set_unset(int_set * table, int key){
  int_set_remove(table, &key, 1);
}

bool int_set_try_get(int_set * table, int * key){
  void * array[] = {(void* )key};
  void * column_pointers[] = {(void *)table->key};
  size_t __index = 0;
  icy_table_finds((icy_table *) table, array[0], &__index, 1);
  if(__index == 0) return false;
  unsigned int sizes[] = {sizeof(int)};
  for(int i = 1; i < 1; i++){
    if(array[i] != NULL)
      memcpy(array[i], column_pointers[i] + __index * sizes[i], sizes[i]); 
  }
  return true;
}

void int_set_print(int_set * table){
  icy_table_print((icy_table *) table);
}

size_t int_set_iter(int_set * table, int * keys, size_t keycnt, int * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator){
  return icy_table_iter((icy_table *) table, keys, keycnt, optional_keys_out, indexes, cnt, iterator);

}
