#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "utils.h"
#include "mem.h"

#include "icydb_int.h"

size_t icy_vector_abs_capacity(icy_vector_abs * table){
  return *table->capacity;
}

size_t icy_vector_abs_count(icy_vector_abs * table){
  return *table->count;
}

void icy_vector_abs_count_set(icy_vector_abs * table, unsigned int newcount){
  *table->count = newcount;
}

size_t _icy_vector_abs_free_index_count(icy_vector_abs * table){
  return *table->free_index_count;
}


void _icy_vector_abs_free_index_count_set(icy_vector_abs * table, size_t cnt){
  *table->free_index_count = cnt;
}


void icy_vector_abs_check_sanity(icy_vector_abs * table){
  size_t cnt = _icy_vector_abs_free_index_count(table);
  ASSERT(table->free_indexes->size / sizeof(size_t) > cnt);
}


void icy_vector_abs_clear(icy_vector_abs * table){
  icy_vector_abs_count_set(table, 0);
  _icy_vector_abs_free_index_count_set(table, 0);
  icy_mem_realloc(table->free_indexes, sizeof(size_t));
}

void ** get_ptrs(icy_vector_abs * table){
  void * sizes_ptr = &table->tail;
  void ** ptrs = sizes_ptr + sizeof(size_t) * table->column_count;
  return ptrs;
}

icy_mem ** get_mem_areas(icy_vector_abs * table){
  void * sizes_ptr = &table->tail;
  void ** ptrs = sizes_ptr + sizeof(size_t) * table->column_count;
  icy_mem ** areas = (icy_mem **) &ptrs[table->column_count];
  return areas;
}

size_t * get_column_sizes(icy_vector_abs * table){
  return (size_t *) &table->tail;  
}

void icy_vector_abs_set_capacity(icy_vector_abs * table, size_t newcapacity){
  icy_mem ** areas = get_mem_areas(table);
  void ** ptrs = get_ptrs(table);
  size_t * column_sizes = get_column_sizes(table);
  for(size_t i = 0; i < table->column_count; i++){

    size_t prevsize = areas[i]->size; 
    //ASSERT((prevsize % table->element_size) == 0);
    size_t newsize = column_sizes[i] * newcapacity;
    icy_mem_realloc(areas[i], newsize);
    if(newsize > prevsize)
      memset(areas[i]->ptr + prevsize, 0,  newsize - prevsize);
    ptrs[i] = areas[i]->ptr;
  }
  *table->capacity = newcapacity;
}


void icy_vector_abs_increase_capacity(icy_vector_abs * table){
  icy_vector_abs_set_capacity(table, MAX((size_t)8, *table->capacity * 2));
}


size_t _icy_vector_abs_alloc(icy_vector_abs * table){
  size_t freeindexcnt = _icy_vector_abs_free_index_count(table);
  logd("Freeidx cnt: %i\n", freeindexcnt);
  if(freeindexcnt > 0){
    size_t idx = ((size_t *) table->free_indexes->ptr)[freeindexcnt];
    _icy_vector_abs_free_index_count_set(table, freeindexcnt - 1);
    //void * p = icy_vector_abs_lookup(table, (icy_index){idx});
    //memset(p, 0, table->element_size);
    return idx;
  }
  
  while(icy_vector_abs_capacity(table) <= icy_vector_abs_count(table)){
    icy_vector_abs_increase_capacity(table);
  }
  
  size_t idx = icy_vector_abs_count(table);
  *table->count += 1;
  return idx;
}

icy_index icy_vector_abs_alloc(icy_vector_abs * table){
  auto index = _icy_vector_abs_alloc(table);
  return (icy_index){index};
}

void icy_vector_abs_remove(icy_vector_abs * table, icy_index index){
  ASSERT(index.index < icy_vector_abs_count(table));
  size_t cnt = _icy_vector_abs_free_index_count(table);
  icy_mem_realloc(table->free_indexes, table->free_indexes->size + sizeof(size_t));
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), (cnt + 1) * sizeof(size_t), &index, sizeof(index)) == NULL);
  ((size_t *)table->free_indexes->ptr)[cnt + 1] = index.index;
  *table->free_index_count += 1; 
}

void icy_vector_abs_resize_sequence(icy_vector_abs * table, icy_vector_abs_sequence * seq,  size_t new_count){
  icy_vector_abs_check_sanity(table);
  icy_vector_abs_sequence nseq = {0};
  if(new_count == 0){
    icy_vector_abs_remove_sequence(table, seq);
    *seq = nseq;
    return;
  }
  
  nseq = icy_vector_abs_alloc_sequence(table, new_count);

  if(seq->index.index != 0 && seq->count != 0 ){
    if(new_count > 0){
      //void * src = icy_vector_abs_lookup_sequence(table, *seq);
      //void * dst = icy_vector_abs_lookup_sequence(table, nseq);
      //memmove(dst, src, MIN(seq->count, nseq.count) * table->element_size);
    }
    icy_vector_abs_remove_sequence(table, seq);
  }
  *seq = nseq;
  icy_vector_abs_check_sanity(table);
}



icy_vector_abs_sequence icy_vector_abs_alloc_sequence(icy_vector_abs * table, size_t  count){
  size_t freeindexcnt = _icy_vector_abs_free_index_count(table);
  
  if(freeindexcnt > 0){
      size_t start = 0;
      size_t cnt = 0;
      for(size_t i = 0; i < freeindexcnt; i++){
	size_t idx = ((size_t *) table->free_indexes->ptr)[i + 1];
	if(start == 0){
	  start = idx;
	  cnt = 1;
	}else if(idx == start + cnt){
	  cnt += 1;
	}else if(idx == start + cnt - 1){
	  ASSERT(false); // it seems that some bug can appear here.
	}else{
	  start = idx;
	  cnt = 1;
	}
	if(cnt == count){
	  // pop it from the indexex.
	  _icy_vector_abs_free_index_count_set(table, freeindexcnt - cnt);
	  size_t * ptr = table->free_indexes->ptr;
	  for(size_t j = i - cnt + 1; j < freeindexcnt - cnt; j++)
	    ptr[j + 1] = ptr[j + cnt + 1];
	  
	  ASSERT(start != 0);
	  //void * p = icy_vector_abs_lookup(table, (icy_index){start});
	  //memset(p, 0, cnt * table->element_size);
	  return (icy_vector_abs_sequence){.index = (icy_index){start}, .count = cnt};
	}
      }
    }
  
  while(icy_vector_abs_capacity(table) <= (icy_vector_abs_count(table) + count)){
    icy_vector_abs_increase_capacity(table);
  }
  size_t idx = icy_vector_abs_count(table);
  icy_vector_abs_count_set(table,idx + count);
  return (icy_vector_abs_sequence){.index = (icy_index){idx}, .count = count};
}

void icy_vector_abs_remove_sequence(icy_vector_abs * table, icy_vector_abs_sequence * seq){
  size_t cnt = _icy_vector_abs_free_index_count(table);
  icy_mem_realloc(table->free_indexes, table->free_indexes->size + seq->count * sizeof(size_t));
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), cnt * sizeof(size_t), &index, sizeof(index)) == NULL);
  for(size_t i = 0; i < seq->count; i++)
    ((size_t *)table->free_indexes->ptr)[cnt + i + 1] = seq->index.index + i;
  *table->free_index_count += seq->count;
  memset(seq, 0, sizeof(*seq));
  icy_vector_abs_optimize(table);
}

typedef struct{
  size_t count;
  size_t capacity;
  size_t free_index_count;
}icy_abstract_vector_header;

void icy_vector_abs_init(icy_vector_abs * base, const char * name){
  //ASSERT(element_size > 0);

  if(name != NULL){
    char name2[128] = {0};     
    sprintf(name2, "%s.free", name);
    base->free_indexes = icy_mem_create(name2);

    sprintf(name2, "%s.__header__", name);
    base->header = icy_mem_create(name2);
    icy_mem ** areas = get_mem_areas(base);
    for(size_t i = 0; i < base->column_count; i++){
      memset(name2, 0, sizeof(name2));
      sprintf(name2, "%s.%s",name, base->column_names[i]);
      areas[i] = icy_mem_create(name2);
    }
    
  }else{
    base->free_indexes = icy_mem_create3();
    base->header = icy_mem_create3();
    icy_mem ** areas = get_mem_areas(base);
    for(size_t i = 0; i < base->column_count; i++){
      areas[i] = icy_mem_create3();
    }
  }
  
  if(base->header->size < sizeof(icy_abstract_vector_header)){
    icy_mem_realloc(base->header, sizeof(icy_abstract_vector_header));
    memset(base->header->ptr, 0, sizeof(icy_abstract_vector_header));
  }
  icy_abstract_vector_header * header = base->header->ptr;
  
  base->count = &header->count;
  base->capacity = &header->capacity;
  base->free_index_count = &header->free_index_count;
  
  
  icy_mem ** areas = get_mem_areas(base);
  if(base->column_count > 0 && areas[0]->size == 1)
    icy_vector_abs_set_capacity(base, 8);
  void ** ptrs = get_ptrs(base);
  //size_t * sizes = get_column_sizes(base);
  for(size_t i = 0; i < base->column_count; i++){
    ptrs[i] = areas[i]->ptr;
    //ASSERT((sizes[i] == 0 ||  areas[i]->size % sizes[i]) == 0);
  }
  
  /*
  if(table.area->size < element_size){
    icy_mem_realloc(table.area, element_size * 4);
    memset(table.area->ptr, 0, table.area->size);
    _icy_vector_abs_alloc(&table);
    }*/

}


void icy_vector_abs_destroy(icy_vector_abs ** _table){
  ERROR("Not implemented");
  icy_vector_abs * table = *_table;
  *_table = NULL;
  //icy_mem_free(table->area);
  icy_mem_free(table->free_indexes);
}

bool icy_vector_abs_contains(icy_vector_abs * table, icy_index index){
  if(index.index == 0)
    return false;
  if (index.index >= icy_vector_abs_count(table))
    return false;
  size_t freecnt = _icy_vector_abs_free_index_count(table);
  size_t * start = table->free_indexes->ptr + sizeof(size_t);
  for(size_t i = 0; i < freecnt; i++){
    if(start[i] == index.index)
      return false;
  }
  return true;
}

static int cmpfunc (const size_t * a, const size_t * b)
  {
    return ( *(int*)a - *(int*)b );
  }

void icy_vector_abs_optimize(icy_vector_abs * table){
  // the index table is optimized by sorting and removing excess elements
  //
  
  size_t free_cnt = _icy_vector_abs_free_index_count(table);
  if(table->free_indexes->ptr == NULL || free_cnt ==  0)
    return; // nothing to optimize.
    
  size_t * p = table->free_indexes->ptr;
  qsort(p + 1,free_cnt , sizeof(size_t), (void *) cmpfunc);
  size_t table_cnt = icy_vector_abs_count(table);

  // remove all elements that are bigger than the table. note: how did these enter?
  while(p[free_cnt] >= table_cnt && free_cnt != 0){
    free_cnt -= 1;
  }

  // remove all elements that are freed from the end of the table
  while(table_cnt > 0 && p[free_cnt] == table_cnt - 1 && free_cnt != 0){
    table_cnt -= 1;
    free_cnt -= 1;
  }
  _icy_vector_abs_free_index_count_set(table, free_cnt);
  icy_vector_abs_count_set(table, table_cnt);
  icy_vector_abs_set_capacity(table, table_cnt);
  
  size_t newsize2 = (1 + free_cnt) * sizeof(size_t);
  icy_mem_realloc(table->free_indexes, newsize2);
  icy_vector_abs_check_sanity(table);
}
