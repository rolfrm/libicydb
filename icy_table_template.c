// This file is auto generated by icy-table.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif


TABLE_NAME * TABLE_NAME_create(const char * optional_name){
  static const char * const column_names[] = COLUMN_NAMES;
  static const char * const column_types[] = COLUMN_TYPE_NAMES;
  TABLE_NAME * instance = calloc(sizeof(TABLE_NAME), 1);
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  icy_table_init((icy_table * )instance, optional_name, COLUMN_COUNT, (unsigned int[])COLUMN_SIZES, (char *[])COLUMN_NAMES);
  
  return instance;
}

void TABLE_NAME_insert(TABLE_NAME * table, VALUE_COLUMNS3, size_t count){
  void * array[] = {VALUE_PTRS2};
  icy_table_inserts((icy_table *) table, array, count);
}

void TABLE_NAME_set(TABLE_NAME * table, VALUE_COLUMNS2){
  void * array[] = {VALUE_PTRS1};
  icy_table_inserts((icy_table *) table, array, 1);
}

void TABLE_NAME_lookup(TABLE_NAME * table, INDEX_TYPE * keys, size_t * out_indexes, size_t count){
  icy_table_finds((icy_table *) table, keys, out_indexes, count);
}

void TABLE_NAME_remove(TABLE_NAME * table, INDEX_TYPE * keys, size_t key_count){
  size_t indexes[key_count];
  size_t index = 0;
  size_t cnt = 0;
  while(0 < (cnt = icy_table_iter((icy_table *) table, keys, key_count, NULL, indexes, array_count(indexes), &index))){
    icy_table_remove_indexes((icy_table *) table, indexes, cnt);
    index = 0;
  }
}

void TABLE_NAME_clear(TABLE_NAME * table){
  icy_table_clear((icy_table *) table);
}

void TABLE_NAME_unset(TABLE_NAME * table, INDEX_TYPE key){
  TABLE_NAME_remove(table, &key, 1);
}

bool TABLE_NAME_try_get(TABLE_NAME * table, VALUE_COLUMNS3){
  void * array[] = {VALUE_PTRS2};
  void * column_pointers[] = COLUMN_POINTERS;
  size_t __index = 0;
  icy_table_finds((icy_table *) table, array[0], &__index, 1);
  if(__index == 0) return false;
  unsigned int sizes[] = COLUMN_SIZES;
  for(int i = 1; i < COLUMN_COUNT; i++){
    if(array[i] != NULL)
      memcpy(array[i], column_pointers[i] + __index * sizes[i], sizes[i]); 
  }
  return true;
}

void TABLE_NAME_print(TABLE_NAME * table){
  icy_table_print((icy_table *) table);
}

size_t TABLE_NAME_iter(TABLE_NAME * table, INDEX_TYPE * keys, size_t keycnt, INDEX_TYPE * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator){
  return icy_table_iter((icy_table *) table, keys, keycnt, optional_keys_out, indexes, cnt, iterator);

}
