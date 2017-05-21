
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "types.h"
#include "persist.h"
#include "abstract_sortable.h"
#include "log.h"
#include "index_table.h"
static int keycmp32(const u32 * k1,const  u32 * k2){
  if(*k1 > *k2)
    return 1;
  else if(*k1 == *k2)
    return 0;
  else return -1;
}

static int keycmp(const u64 * k1,const  u64 * k2){
  if(*k1 > *k2)
    return 1;
  else if(*k1 == *k2)
    return 0;
  else return -1;
}

static int keycmp128(const u128 * k1,const  u128 * k2){
  if(*k1 > *k2)
    return 1;
  else if(*k1 == *k2)
    return 0;
  else return -1;
}

static u64 * get_type_sizes(abstract_sorttable * table){
  return (u64 *) &table->tail;
}

static mem_area ** get_mem_areas(abstract_sorttable * table){
  return (mem_area **)(&table->tail + table->column_count + table->column_count);
}

static void ** get_pointers(abstract_sorttable * table){
  return &table->tail + table->column_count;
}

static bool indexes_unique_and_sorted(u64 * indexes, u64 cnt){
  if(cnt == 0) return true;
  for(u64 i = 0; i < cnt - 1; i++)
    if(indexes[i] >= indexes[i + 1])
      return false;
  return true;
}

void abstract_sorttable_check_sanity(abstract_sorttable * table){
  mem_area ** areas = get_mem_areas(table);
  u64 * type_size = get_type_sizes(table);
  u64 cnt = 0;
  for(u32 i = 0; i < table->column_count; i++){
    if(cnt == 0)
      cnt = areas[i]->size / type_size[i];
    else
      ASSERT(cnt == areas[i]->size / type_size[i]);
  }
}
void * bsearch_bigger(int (*cmp)(void*, void*), void * key, void * pt, void * end, size_t keysize);
  
bool abstract_sorttable_keys_sorted(abstract_sorttable * table, void * keys, u64 cnt){
  u64 key_size = get_type_sizes(table)[0];
  if(cnt == 0) return true;
  if(table->is_multi_table){
    for(u64 i = 0; i < cnt - 1; i++)
      if(table->cmp(keys + (i * key_size), keys + (i + 1) * key_size) > 0)
	return false;
  }else{
    for(u64 i = 0; i < cnt - 1; i++)
      if(table->cmp(keys + (i * key_size), keys + (i + 1) * key_size) >= 0)
	return false;
  }
  return true;
}

void abstract_sorttable_init(abstract_sorttable * table,const char * table_name , u32 column_count, u32 * column_size, char ** column_name){
  char pathbuf[100];
  *((u32 *)(&table->column_count)) = column_count;
  mem_area ** mem_areas = get_mem_areas(table);
  void ** pointers = get_pointers(table);
  u64 * type_sizes = get_type_sizes(table);
  u64 key_size = column_size[0];
  if(key_size == sizeof(u128))
    table->cmp = (void *) keycmp128;
  else if(key_size == sizeof(u32)){

    table->cmp = (void *) keycmp32;
  }
  else
    table->cmp = (void *) keycmp;
  

  for(u32 i = 0; i < column_count; i++){
    type_sizes[i] = column_size[i];
    if(column_name[i] == NULL || table_name == NULL){
      mem_areas[i] = create_non_persisted_mem_area();
    }else{
      sprintf(pathbuf, "table2/%s.%s", table_name, column_name[i]);
      mem_areas[i] = create_mem_area(pathbuf);
    }
    if(mem_areas[i]->size < type_sizes[i]){
      if(i > 0 && mem_areas[0]->size > type_sizes[0]){
	mem_area_realloc(mem_areas[i], type_sizes[i] * (mem_areas[0]->size / type_sizes[0]));
      }else{
	mem_area_realloc(mem_areas[i], type_sizes[i]);
      }
      memset(mem_areas[i]->ptr, 0, mem_areas[i]->size);
    }
    
    pointers[i] = mem_areas[i]->ptr;
  }
  table->count = mem_areas[0]->size / key_size - 1;
  abstract_sorttable_check_sanity(table);
}

void abstract_sorttable_finds(abstract_sorttable * table, void * keys, u64 * indexes, u64 cnt){
  ASSERT(abstract_sorttable_keys_sorted(table, keys, cnt));
  abstract_sorttable_check_sanity(table);
  memset(indexes, 0, cnt * sizeof(indexes[0]));
  u64 key_size = get_type_sizes(table)[0];
  mem_area * key_area = get_mem_areas(table)[0];
  
  if(key_area->size <= key_size)
    return;
  void * start = key_area->ptr + key_size;
  void * end = key_area->ptr + key_area->size;
  for(u64 i = 0; i < cnt || indexes == NULL; i++){

    //if(end < start) break;
    u64 size = end - start;
    void * key_index = NULL;
    void * key = keys + i * key_size;
    int startcmp = table->cmp(key, start);
    if(startcmp < 0) continue;
    if(startcmp == 0)
      key_index = start;
    else if(table->cmp(key, end - key_size) > 0){
      return;
    }else
      //key_index =memmem(start,size,key,table->key_size);
      key_index = bsearch(key, start, size / key_size, key_size, (void *)table->cmp);
    
    if(key_index == 0){
      if(indexes != NULL)
	indexes[i] = 0;
    }else{
      if(indexes != NULL)
	indexes[i] = (key_index - key_area->ptr) / key_size;
      start = key_index + key_size;
    }    
  }
}


void abstract_sorttable_insert_keys(abstract_sorttable * table, void * keys, u64 * out_indexes, u64 cnt){
  ASSERT(abstract_sorttable_keys_sorted(table, keys, cnt));
  abstract_sorttable_check_sanity(table);
  u64 * column_size = get_type_sizes(table);
  mem_area ** column_area = get_mem_areas(table);
  void ** pointers = get_pointers(table);
  mem_area * key_area = column_area[0];
  u64 key_size = column_size[0];
  u32 column_count = table->column_count;
  // make room for new data.
  for(u32 i = 0; i < column_count; i++){
    mem_area_realloc(column_area[i], column_area[i]->size + cnt * column_size[i]);
    pointers[i] = column_area[i]->ptr;
  }
  
  // skip key related things
  column_size += 1;
  column_area += 1;
  column_count -= 1;

  abstract_sorttable_check_sanity(table);  
  void * pt = key_area->ptr + key_size;
  void * end = key_area->ptr + key_area->size - key_size * cnt;

  void * vend[column_count];
  for(u32 i = 0; i < column_count; i++)
    vend[i] = column_area[i]->ptr + column_area[i]->size - column_size[i] * cnt;
  int (*cmp)( void*,  void*) = table->cmp;
  for(u64 i = 0; i < cnt; i++){
    pt = bsearch_bigger((void *)cmp, keys, pt, end, key_size);
    while(pt < end && cmp(pt, keys) <= 0)
      pt += key_size;
    
    u64 offset = (pt - key_area->ptr) / key_size;
    // move everything from keysize up
    memmove(pt + key_size, pt , end - pt); 
    memmove(pt, keys, key_size);

    for(u32 j = 0; j < column_count; j++){
      void * vpt = column_area[j]->ptr + offset * column_size[j];
      memmove(vpt + column_size[j], vpt, vend[j] - vpt);
      memset(vpt, 0, column_size[j]);
    }
  
    *out_indexes = (pt - key_area->ptr) / key_size;
    
    out_indexes += 1;
    keys += key_size;
    pt += key_size;
    end += key_size;
    for(u32 j = 0; j < table->column_count; j++){
      vend[j] += column_size[j];
    }
  }
  table->count = key_area->size / key_size - 1;
  //todo: disable then when tables gets big enough
  // until then this will detect possible programming errors, causing the tables to rapidly expand.
  ASSERT(table->count < 100000);
}

void abstract_sorttable_inserts(abstract_sorttable * table, void ** values, u64 cnt){
  abstract_sorttable_check_sanity(table);
  ASSERT(values[0] != NULL);
  void * keys = values[0];

  u64 * column_size = get_type_sizes(table);
  mem_area ** column_area = get_mem_areas(table);
  
  ASSERT(abstract_sorttable_keys_sorted(table, keys, cnt));
  u64 indexes[cnt];
  memset(indexes, 0, sizeof(indexes));
  u64 newcnt = 0;
  if(table->is_multi_table){
    // overwrite is never done for multi tables.
    newcnt = cnt;
  }else{
    abstract_sorttable_finds(table, keys, indexes, cnt);
    for(u64 i = 0; i < cnt ; i++){
      if(indexes[i] == 0)
	newcnt += 1;
    }
    if(newcnt != cnt){
      for(u32 j = 1; j < table->column_count; j++){
	mem_area * value_area = column_area[j];
	u32 size = column_size[j];
	for(u64 i = 0; i < cnt; i++){
	  if(indexes[i] != 0){
	    // overwrite existing values with new values
	    memcpy(value_area->ptr + size * indexes[i], values[j] + size * i, size);
	  }
	}
      }
    }
  }
  u64 indexes2[newcnt];
  memset(indexes2, 0, sizeof(indexes2));
  {
    u32 csize = column_size[0];
    void * newvalues = calloc(newcnt, csize);
    u64 offset = 0;
    for(u64 i = 0; i < cnt; i++){
      if(indexes[i] == 0){
	indexes2[offset] = i;
	memcpy(newvalues + csize * offset, values[0] + i * csize, csize);
	offset += 1;
      }
    }
    memset(indexes, 0, sizeof(indexes));
  // make room and insert keys
    abstract_sorttable_insert_keys(table, newvalues, indexes, newcnt);
  }

  // Insert the new data
  for(u32 j = 1; j < table->column_count; j++){
    u64 csize = column_size[j];
    for(u64 i = 0; i < newcnt; i++){
      u64 idx = indexes2[i];
      ASSERT(indexes[i]);
      memcpy(column_area[j]->ptr + csize * indexes[i], values[j] + csize * idx, csize);
    }
  }
}

void abstract_sorttable_clear(abstract_sorttable * table){
  u64 * column_size = get_type_sizes(table);
  mem_area ** column_area = get_mem_areas(table);
  void ** pointers = get_pointers(table);
  
  for(u32 i = 0; i < table->column_count; i++){
    mem_area_realloc(column_area[i], column_size[i]);
    pointers[i] = column_area[i]->ptr;
  }
  table->count = 0;
}

void abstract_sorttable_remove_indexes(abstract_sorttable * table, u64 * indexes, size_t cnt){
  ASSERT(indexes_unique_and_sorted(indexes, cnt));

  u64 * column_size = get_type_sizes(table);
  mem_area ** column_area = get_mem_areas(table);
  void ** pointers = get_pointers(table);
  
  const u64 _table_cnt = column_area[0]->size / column_size[0];
  for(u32 j = 0; j < table->column_count; j++){
    u64 table_cnt = _table_cnt;
    void * pt = column_area[j]->ptr;
    u64 size = column_size[j];

    for(u64 _i = 0; _i < cnt; _i++){
      u64 i = cnt - _i - 1;
      u64 index = indexes[i];
      memmove(pt + index * size, pt + (1 + index) * size, (table_cnt - index - 1) * size);
      table_cnt--;
    }
    mem_area_realloc(column_area[j], table_cnt * size);
    pointers[j] = column_area[j]->ptr;
  }
  table->count = column_area[0]->size / column_size[0] - 1;
  abstract_sorttable_check_sanity(table);
  
}

void table_print_cell(void * ptr, const char * type);
void abstract_sorttable_print(abstract_sorttable * table){
  void ** pointers = get_pointers(table);
  u64 * sizes = get_type_sizes(table);
  for(u32 i = 0; i < table->column_count;i++)
    logd("%s ", table->column_types[i]);
  logd("    rows: %i\n", table->count);
  for(u32 i = 0; i < table->count; i++){
    for(u32 j = 0; j < table->column_count;j++){
      table_print_cell(pointers[j] + (1 + i) * sizes[j], table->column_types[j]);
      logd(" ");
    }
    logd("\n");
  }
}

size_t abstract_sorttable_iter(abstract_sorttable * table, void * keys, size_t keycnt, void * out_keys, u64 * indexes, size_t cnt, size_t * idx){
  u64 fakeidx = 0;
  if(idx == NULL)
    idx = &fakeidx;
  
  u64 key_size = get_type_sizes(table)[0];
  mem_area * key_area = get_mem_areas(table)[0];
  
  u64 orig_cnt = cnt;
  if(*idx == 0) *idx = 1;
  for(u64 i = 0; i < keycnt; i++){
    void * key = keys + i * key_size;
    void * start = key_area->ptr + *idx * key_size;
    void * end = key_area->ptr + key_area->size;
    if(start >= end)
      break;
    u64 size = end - start;
    void * key_index = NULL;
    int firstcmp = table->cmp(key, start);
    if(firstcmp < 0) 
      continue; // start is bigger than key
    if(firstcmp == 0) 
      key_index = start; // no need to search.
    else
      key_index = memmem(start, size, key, key_size);
    //key_index = bsearch(key, start, size / key_size, key_size, table->cmp);
    if(key_index == NULL)
      continue;
    start = key_index;
    *idx = (start - key_area->ptr) / key_size;
    
    do{
      if(out_keys != NULL){
	memcpy(out_keys, key, key_size);
	out_keys += key_size;
      }
      if(indexes != NULL){
	*indexes = *idx;
	indexes += 1;
      }
      cnt -= 1;
      start += key_size;
      *idx += 1; 
    }while(table->cmp(start, key) == 0 && cnt > 0);

  }
  return orig_cnt - cnt;
}

void * bsearch_bigger(int (*cmp)(void*, void*), void * key, void * pt, void * end, size_t keysize){

  u64 a = 0;
  u64 cnt = ((u64)(end - pt)) / keysize;
  if(cnt == 0) return pt;
  if(cmp(pt + a, key) > 0)
    return pt;
  u64 b = cnt - 1;

  
  while(a != b){
    u32 c = (a + b) / 2;
    if(cmp(pt + c * keysize, key) > 0)
      b = c;
    else
      a = c + 1;
  }
  return pt + a * keysize;
}



bool pu64(u64 * p, const char * type){
      if(strcmp(type, "u64") == 0){
	
	logd("%i", *p);
	return true;
      }
      return false;
    }
bool pu32(u32 * p, const char * type){
  if(strcmp(type, "u32") == 0 | 0 == strcmp(type, "int")){
    logd("%i", *p);
    return true;
  }
  return false;
}

bool pf32(f32 * p, const char * type){
  if(strcmp(type, "f32") == 0){
    logd("%f", *p);
    return true;
  }
  return false;
}

bool (** printer_table)(void * ptr, const char * type) = NULL ;
size_t printer_table_cnt = 0;

index_table * init_printers(){
  static index_table * printers = NULL;
  if(printers == NULL){
    printers = index_table_create(NULL, sizeof(printer_table));
    
    add_table_printer((void *)pu64);
    add_table_printer((void *)pu32);
    add_table_printer((void *)pf32);
  }
  return printers;
}

void add_table_printer(bool (*printer)(void * ptr, const char * type)){
  index_table * printers = init_printers();
  size_t indx = index_table_alloc(printers);
  ((void **)index_table_lookup(printers, indx))[0] = printer;
  printer_table = index_table_all(printers, &printer_table_cnt);
}

void table_print_cell(void * ptr, const char * type){
  init_printers();
  
  for(size_t i = 0; i < printer_table_cnt; i++){
    if(printer_table[i](ptr, type))
      return;
  }
  logd("%s", type);
}
