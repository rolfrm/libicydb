#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "types.h"
#include "log.h"
#include "icydb_int.h"

static int keycmpf64(const f64 * k1,const  f64 * k2){
  if(*k1 > *k2)
    return 1;
  else if(*k1 == *k2)
    return 0;
  else return -1;
}

static int keycmpf32(const f32 * k1,const  f32 * k2){
  if(*k1 > *k2)
    return 1;
  else if(*k1 == *k2)
    return 0;
  else return -1;
}

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

/*
static int keycmp128(const u128 * k1,const  u128 * k2){
  if(k1->a > k2->a)
    return 1;
  if(k1->a < k2->a)
    return -1;
  return keycmp(&k1->b, &k2->b);
  }*/

__thread int dyncmp_size = 0;
static int keycmpdyn(const u8 * k1, const u8 * k2){
  ASSERT(dyncmp_size);
  return memcmp(k1, k2, dyncmp_size);
}

static size_t * get_type_sizes(icy_table * table){
  return (size_t *) &table->tail;
}

static icy_mem ** get_icy_mems(icy_table * table){
  return (icy_mem **)(&table->tail + table->column_count + table->column_count);
}

static void ** get_pointers(icy_table * table){
  return &table->tail + table->column_count;
}

static bool indexes_unique_and_sorted(size_t * indexes, size_t cnt){
  if(cnt == 0) return true;
  for(size_t i = 0; i < cnt - 1; i++)
    if(indexes[i] >= indexes[i + 1])
      return false;
  return true;
}

void icy_table_check_sanity(icy_table * table){
  icy_mem ** areas = get_icy_mems(table);
  size_t * type_size = get_type_sizes(table);
  size_t cnt = 0;
  for(u32 i = 0; i < table->column_count; i++){
    if(cnt == 0)
      cnt = areas[i]->size / type_size[i];
    else
      ASSERT(cnt == areas[i]->size / type_size[i]);
  }
}
void * bsearch_bigger(int (*cmp)(void*, void*), void * key, void * pt, void * end, size_t keysize);
  
bool icy_table_keys_sorted(icy_table * table, void * keys, size_t cnt){
  size_t key_size = get_type_sizes(table)[0];
  if(cnt == 0) return true;
  if(table->is_multi_table){
    for(size_t i = 0; i < cnt - 1; i++)
      if(table->cmp(keys + (i * key_size), keys + (i + 1) * key_size) > 0)
	return false;
  }else{
    for(size_t i = 0; i < cnt - 1; i++)
      if(table->cmp(keys + (i * key_size), keys + (i + 1) * key_size) >= 0)
	return false;
  }
  return true;
}

void icy_table_init(icy_table * table,const char * table_name , u32 column_count, u32 * column_size, char ** column_name){
  char pathbuf[100];
  *((u32 *)(&table->column_count)) = column_count;
  icy_mem ** icy_mems = get_icy_mems(table);
  void ** pointers = get_pointers(table);
  size_t * type_sizes = get_type_sizes(table);
  size_t key_size = column_size[0];
  if(column_count > 0 && table->column_types != NULL
	  && (strcmp(table->column_types[0], "f32") == 0
	      || strcmp(table->column_types[0], "float") == 0))
    table->cmp = (void *) keycmpf32;
  else if(column_count > 0 && table->column_types != NULL
	  && (strcmp(table->column_types[0], "f64") == 0
	      || strcmp(table->column_types[0], "double") == 0))
    table->cmp = (void *) keycmpf64;  
  else if(key_size == sizeof(u32))
    table->cmp = (void *) keycmp32;
  else if(key_size == sizeof(u64))
    table->cmp = (void *) keycmp;
  else
    table->cmp = (void *) keycmpdyn;
  

  for(u32 i = 0; i < column_count; i++){
    type_sizes[i] = column_size[i];
    if(column_name[i] == NULL || table_name == NULL){
      icy_mems[i] = icy_mem_create3();
    }else{
      sprintf(pathbuf, "table2/%s.%s", table_name, column_name[i]);
      icy_mems[i] = icy_mem_create(pathbuf);
    }
    if(icy_mems[i]->size < type_sizes[i]){
      if(i > 0 && icy_mems[0]->size > type_sizes[0]){
	icy_mem_realloc(icy_mems[i], type_sizes[i] * (icy_mems[0]->size / type_sizes[0]));
      }else{
	icy_mem_realloc(icy_mems[i], type_sizes[i]);
      }
      memset(icy_mems[i]->ptr, 0, icy_mems[i]->size);
    }
    
    pointers[i] = icy_mems[i]->ptr;
  }
  table->count = icy_mems[0]->size / key_size - 1;
  icy_table_check_sanity(table);
}

void icy_table_finds(icy_table * table, void * keys, size_t * indexes, size_t cnt){
  ASSERT(icy_table_keys_sorted(table, keys, cnt));
  icy_table_check_sanity(table);
  memset(indexes, 0, cnt * sizeof(indexes[0]));
  size_t key_size = get_type_sizes(table)[0];
  icy_mem * key_area = get_icy_mems(table)[0];
  
  if(key_area->size <= key_size)
    return;
  void * start = key_area->ptr + key_size;
  void * end = key_area->ptr + key_area->size;
  dyncmp_size = key_size;
  for(size_t i = 0; i < cnt || indexes == NULL; i++){

    //if(end < start) break;
    size_t size = end - start;
    void * key_index = NULL;
    void * key = keys + i * key_size;
    int startcmp = table->cmp(key, start);
    if(startcmp < 0) continue;
    if(startcmp == 0)
      key_index = start;
    else if(table->cmp(key, end - key_size) > 0){
      dyncmp_size = 0;
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
  dyncmp_size = 0;
}


void icy_table_insert_keys(icy_table * table, void * keys, size_t * out_indexes, size_t cnt){
  size_t * orig_out_indexes = out_indexes;
  ASSERT(icy_table_keys_sorted(table, keys, cnt));
  icy_table_check_sanity(table);
  size_t * column_size = get_type_sizes(table);
  icy_mem ** column_area = get_icy_mems(table);
  void ** pointers = get_pointers(table);
  icy_mem * key_area = column_area[0];
  size_t key_size = column_size[0];
  u32 column_count = table->column_count;
  // make room for new data.
  for(u32 i = 0; i < column_count; i++){
    icy_mem_realloc(column_area[i], column_area[i]->size + cnt * column_size[i]);
    pointers[i] = column_area[i]->ptr;
  }
  
  // skip key related things
  column_size += 1;
  column_area += 1;
  column_count -= 1;

  icy_table_check_sanity(table);  
  void * pt = key_area->ptr + key_size;
  void * end = key_area->ptr + key_area->size - key_size * cnt;

  void * vend[column_count];
  for(u32 i = 0; i < column_count; i++)
    vend[i] = column_area[i]->ptr + column_area[i]->size - column_size[i] * cnt;
  int (*cmp)( void*,  void*) = table->cmp;
  dyncmp_size = key_size;
  for(size_t i = 0; i < cnt; i++){
    pt = bsearch_bigger((void *)cmp, keys, pt, end, key_size);
    while(pt < end && cmp(pt, keys) <= 0)
      pt += key_size;
    
    size_t offset = (pt - key_area->ptr) / key_size;
    // move everything from keysize up
    memmove(pt + key_size, pt , end - pt); 
    memmove(pt, keys, key_size);

    for(u32 j = 0; j < column_count; j++){
      void * vpt = column_area[j]->ptr + offset * column_size[j];
      memmove(vpt + column_size[j], vpt, vend[j] - vpt);
      memset(vpt, 0, column_size[j]);
    }
    size_t testcnt = out_indexes - orig_out_indexes;
    ASSERT(testcnt <= cnt);
    //logd("Write.. %i\n", testcnt);
    // TODO: Fix issue apparantly arising with emcc and O>0.
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
  dyncmp_size = 0;
}


void icy_table_inserts(icy_table * table, void ** values, size_t cnt){
  icy_table_check_sanity(table);
  ASSERT(values[0] != NULL);
  void * keys = values[0];

  size_t * column_size = get_type_sizes(table);
  icy_mem ** column_area = get_icy_mems(table);
  ASSERT(icy_table_keys_sorted(table, keys, cnt));
  size_t indexes[cnt];
  memset(indexes, 0, sizeof(indexes));
  size_t newcnt = 0;
  if(table->is_multi_table){
    // overwrite is never done for multi tables.
    newcnt = cnt;
  }else{
    icy_table_finds(table, keys, indexes, cnt);
    for(size_t i = 0; i < cnt ; i++){
      if(indexes[i] == 0)
	newcnt += 1;
    }
    if(newcnt != cnt){
      for(u32 j = 1; j < table->column_count; j++){
	icy_mem * value_area = column_area[j];
	u32 size = column_size[j];
	for(size_t i = 0; i < cnt; i++){
	  if(indexes[i] != 0){
	    // overwrite existing values with new values
	    memcpy(value_area->ptr + size * indexes[i], values[j] + size * i, size);
	  }
	}
      }
    }
  }
   size_t indexes2[newcnt];
  memset(indexes2, 0, sizeof(indexes2));
  {
    u32 csize = column_size[0];

    static __thread void * newvalues = NULL;
    static __thread size_t newvalues_size;
    size_t total_size = newcnt * csize;
    if(newvalues_size < total_size){
      newvalues_size = total_size;
      newvalues = realloc(newvalues, newvalues_size);
    }
    memset(newvalues, 0, total_size);
    size_t offset = 0;
    for(size_t i = 0; i < cnt; i++){
      if(indexes[i] == 0){
	indexes2[offset] = i;
	memcpy(newvalues + csize * offset, values[0] + i * csize, csize);

	offset += 1;
      }
    }
    memset(indexes, 0, sizeof(indexes));
  // make room and insert keys
    icy_table_insert_keys(table, newvalues, indexes, newcnt);
  }
  // Insert the new data
  for(u32 j = 1; j < table->column_count; j++){
    size_t csize = column_size[j];
    for(size_t i = 0; i < newcnt; i++){
      size_t idx = indexes2[i];
      ASSERT(indexes[i]);
      memcpy(column_area[j]->ptr + csize * indexes[i], values[j] + csize * idx, csize);
    }
  }
}

void icy_table_clear(icy_table * table){
  size_t * column_size = get_type_sizes(table);
  icy_mem ** column_area = get_icy_mems(table);
  void ** pointers = get_pointers(table);
  
  for(u32 i = 0; i < table->column_count; i++){
    icy_mem_realloc(column_area[i], column_size[i]);
    pointers[i] = column_area[i]->ptr;
  }
  table->count = 0;
}

void icy_table_remove_indexes(icy_table * table, size_t * indexes, size_t cnt){
  ASSERT(indexes_unique_and_sorted(indexes, cnt));

  size_t * column_size = get_type_sizes(table);
  icy_mem ** column_area = get_icy_mems(table);
  void ** pointers = get_pointers(table);
  
  const size_t _table_cnt = column_area[0]->size / column_size[0];
  for(u32 j = 0; j < table->column_count; j++){
    size_t table_cnt = _table_cnt;
    void * pt = column_area[j]->ptr;
    size_t size = column_size[j];

    for(size_t _i = 0; _i < cnt; _i++){
      size_t i = cnt - _i - 1;
      size_t index = indexes[i];
      memmove(pt + index * size, pt + (1 + index) * size, (table_cnt - index - 1) * size);
      table_cnt--;
    }
    icy_mem_realloc(column_area[j], table_cnt * size);
    pointers[j] = column_area[j]->ptr;
  }
  table->count = column_area[0]->size / column_size[0] - 1;
  icy_table_check_sanity(table);
  
}

void icy_print_cell(void * ptr, const char * type);
void icy_table_print(icy_table * table){
  void ** pointers = get_pointers(table);
  size_t * sizes = get_type_sizes(table);
  for(u32 i = 0; i < table->column_count;i++)
    logd("%s ", table->column_types[i]);
  logd("    rows: %i\n", table->count);
  for(u32 i = 0; i < table->count; i++){
    for(u32 j = 0; j < table->column_count;j++){
      icy_print_cell(pointers[j] + (1 + i) * sizes[j], table->column_types[j]);
      logd(" ");
    }
    logd("\n");
  }
}

static void * memmem2(void * start, size_t size, void * key, size_t key_size){
  // like memmem, but only searches chunks of key_size.
  for(size_t i = 0; i < size; i+=key_size){
    if(memcmp(start + i, key, key_size) == 0)
      return start + i;
  }
  return NULL;

}

size_t icy_table_iter(icy_table * table, void * keys, size_t keycnt, void * out_keys, size_t * indexes, size_t cnt, size_t * idx){
  size_t fakeidx = 0;
  if(idx == NULL)
    idx = &fakeidx;
  
  size_t key_size = get_type_sizes(table)[0];
  icy_mem * key_area = get_icy_mems(table)[0];
  dyncmp_size = key_size;
  size_t orig_cnt = cnt;
  if(*idx == 0) *idx = 1;
  for(size_t i = 0; i < keycnt; i++){
    void * key = keys + i * key_size;
    void * start = key_area->ptr + *idx * key_size;
    void * end = key_area->ptr + key_area->size;
    if(start >= end)
      break;
    size_t size = end - start;
    void * key_index = NULL;
    int firstcmp = table->cmp(key, start);
    if(firstcmp < 0) 
      continue; // start is bigger than key
    if(firstcmp == 0) 
      key_index = start; // no need to search.
    else
      {
	if(table->is_multi_table)
	  // if multi table we need to search
	  // for lowest bounds
	  key_index = memmem2(start, size, key, key_size);
	else
	  key_index = bsearch(key, start, size / key_size, key_size, (void *) table->cmp);
      }
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
    }while(cnt > 0 && start < end && table->cmp(start, key) == 0);

  }
  dyncmp_size = 0;
  return orig_cnt - cnt;
}

ICY_HIDDEN void * bsearch_bigger(int (*cmp)(void*, void*), void * key, void * pt, void * end, size_t keysize){

  size_t a = 0;
  size_t cnt = ((size_t)(end - pt)) / keysize;
  if(cnt == 0) return pt;
  if(cmp(pt + a, key) > 0)
    return pt;
  size_t b = cnt - 1;

  
  while(a != b){
    u32 c = (a + b) / 2;
    if(cmp(pt + c * keysize, key) > 0)
      b = c;
    else
      a = c + 1;
  }
  return pt + a * keysize;
}



static bool pu64(u64 * p, const char * type){
  if(strcmp(type, "u64") == 0 || 0 == strcmp(type, "size_t")){
    logd("%i", *p);
    return true;
  }
  return false;
}
static bool pu32(u32 * p, const char * type){
  if(strcmp(type, "u32") == 0 || 0 == strcmp(type, "int")){
    logd("%i", *p);
    return true;
  }
  return false;
}

static bool pf32(f32 * p, const char * type){
  if(strcmp(type, "f32") == 0 || 0 == strcmp(type, "float")){
    logd("%f", *p);
    return true;
  }
  return false;
}

static bool pf64(f64 * p, const char * type){
  if(strcmp(type, "f64") == 0 || 0 == strcmp(type, "double")){
    logd("%f", *p);
    return true;
  }
  return false;
}

static bool (** printer_table)(void * ptr, const char * type) = NULL ;
static size_t printer_table_cnt = 0;

static icy_vector * init_printers(){
  static icy_vector * printers = NULL;
  if(printers == NULL){
    printers = icy_vector_create(NULL, sizeof(printer_table));
    
    icy_table_add((void *)pu64);
    icy_table_add((void *)pu32);
    icy_table_add((void *)pf32);
    icy_table_add((void *)pf64);
  }
  return printers;
}

void icy_table_add(bool (*printer)(void * ptr, const char * type)){
  icy_vector * printers = init_printers();
  icy_index indx = icy_vector_alloc(printers);
  ((void **)icy_vector_lookup(printers, indx))[0] = printer;
  printer_table = icy_vector_all(printers, &printer_table_cnt);
}

void icy_print_cell(void * ptr, const char * type){
  init_printers();
  
  for(size_t i = 0; i < printer_table_cnt; i++){
    if(printer_table[i](ptr, type))
      return;
  }
  logd("%s", type);
}
