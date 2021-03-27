#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "utils.h"
#include "mem.h"

#include "icy_mem.h"
#include "icy_vector.h"


size_t icy_vector_capacity(icy_vector * table){
  return table->area->size / table->element_size - 4;
}

size_t icy_vector_count(icy_vector * table){
  return ((size_t *) table->area->ptr)[0];
}

void icy_vector_count_set(icy_vector * table, unsigned int newcount){
  ((size_t *) table->area->ptr)[0] = newcount;
}

size_t _icy_vector_free_index_count(icy_vector * table){
  return ((size_t *) table->free_indexes->ptr)[0];
}


void _icy_vector_free_index_count_set(icy_vector * table, size_t cnt){
  ((size_t *) table->free_indexes->ptr)[0] = cnt;
}


void icy_vector_check_sanity(icy_vector * table){
  size_t cnt = _icy_vector_free_index_count(table);
  ASSERT(table->free_indexes->size / sizeof(size_t) > cnt);
}


void * icy_vector_all(icy_vector * table, size_t * cnt){
  // return count -1 and a pointer from the placeholder element.
  *cnt = icy_vector_count(table) - 1;
  return table->area->ptr + table->element_size * 5;
}
void icy_vector_clear(icy_vector * table){
  icy_vector_count_set(table, 1);
  _icy_vector_free_index_count_set(table, 0);
  icy_mem_realloc(table->free_indexes, sizeof(size_t));
}

size_t _icy_vector_alloc(icy_vector * table){
  size_t freeindexcnt = _icy_vector_free_index_count(table);
  if(freeindexcnt > 0){
    size_t idx = ((size_t *) table->free_indexes->ptr)[freeindexcnt];
    _icy_vector_free_index_count_set(table, freeindexcnt - 1);
    ASSERT(idx != 0);
    void * p = icy_vector_lookup(table, (icy_index){idx});
    memset(p, 0, table->element_size);
    return idx;
  }
  
  while(icy_vector_capacity(table) <= icy_vector_count(table)){
    size_t prevsize = table->area->size;
    ASSERT((prevsize % table->element_size) == 0);
    size_t newsize = MAX(prevsize * 2, 8 * table->element_size);
    icy_mem_realloc(table->area, newsize);
    memset(table->area->ptr + prevsize, 0,  newsize - prevsize);
  }
  size_t idx = icy_vector_count(table);
  icy_vector_count_set(table,idx + 1);
  return idx;
}

icy_index icy_vector_alloc(icy_vector * table){
  auto index = _icy_vector_alloc(table);
  ASSERT(index > 0);
  return (icy_index){index};
}


void icy_vector_remove(icy_vector * table, icy_index index){
  ASSERT(index.index < icy_vector_count(table));
  ASSERT(index.index > 0);
  size_t cnt = _icy_vector_free_index_count(table);
  icy_mem_realloc(table->free_indexes, table->free_indexes->size + sizeof(size_t));
  ((size_t *)table->free_indexes->ptr)[cnt + 1] = 0;
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), (cnt + 1) * sizeof(size_t), &index, sizeof(index)) == NULL);
  ((size_t *)table->free_indexes->ptr)[cnt + 1] = index.index;
  ((size_t *) table->free_indexes->ptr)[0] += 1;
}

void icy_vector_resize_sequence(icy_vector * table, icy_vector_sequence * seq,  size_t new_count){
  icy_vector_check_sanity(table);
  icy_vector_sequence nseq = {0};
  if(new_count == 0){
    icy_vector_remove_sequence(table, seq);
    *seq = nseq;
    return;
  }
  
  nseq = icy_vector_alloc_sequence(table, new_count);

  if(seq->index.index != 0 && seq->count != 0 ){
    if(new_count > 0){
      void * src = icy_vector_lookup_sequence(table, *seq);
      void * dst = icy_vector_lookup_sequence(table, nseq);
      memmove(dst, src, MIN(seq->count, nseq.count) * table->element_size);
    }
    icy_vector_remove_sequence(table, seq);
  }
  *seq = nseq;
  icy_vector_check_sanity(table);
}

icy_vector_sequence icy_vector_alloc_sequence(icy_vector * table, size_t  count){
  size_t freeindexcnt = _icy_vector_free_index_count(table);
  
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
	  _icy_vector_free_index_count_set(table, freeindexcnt - cnt);
	  size_t * ptr = table->free_indexes->ptr;
	  for(size_t j = i - cnt + 1; j < freeindexcnt - cnt; j++)
	    ptr[j + 1] = ptr[j + cnt + 1];
	  
	  ASSERT(start != 0);
	  void * p = icy_vector_lookup(table, (icy_index){start});
	  memset(p, 0, cnt * table->element_size);
	  return (icy_vector_sequence){.index = (icy_index){start}, .count = cnt};
	}
      }
    }
  
  while(icy_vector_capacity(table) <= (icy_vector_count(table) + count)){
    size_t prevsize = table->area->size;
    ASSERT((prevsize % table->element_size) == 0);
    size_t newsize = MAX(prevsize * 2, 8 * table->element_size);
    icy_mem_realloc(table->area, newsize);
    memset(table->area->ptr + prevsize, 0,  newsize - prevsize);
  }
  size_t idx = icy_vector_count(table);
  icy_vector_count_set(table,idx + count);
  return (icy_vector_sequence){.index = (icy_index){idx}, .count = count};
}

void icy_vector_remove_sequence(icy_vector * table, icy_vector_sequence * seq){
  size_t cnt = _icy_vector_free_index_count(table);
  icy_mem_realloc(table->free_indexes, table->free_indexes->size + seq->count * sizeof(size_t));
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), cnt * sizeof(size_t), &index, sizeof(index)) == NULL);
  for(size_t i = 0; i < seq->count; i++)
    ((size_t *)table->free_indexes->ptr)[cnt + i + 1] = seq->index.index + i;
  ((size_t *) table->free_indexes->ptr)[0] += seq->count;
  memset(seq, 0, sizeof(*seq));
  icy_vector_optimize(table);
}

void * icy_vector_lookup_sequence(icy_vector * table, icy_vector_sequence seq){
  if(seq.index.index == 0)
    return NULL;
  return icy_vector_lookup(table, seq.index);
}


icy_vector * icy_vector_create(const char * name, size_t element_size){
  ASSERT(element_size > 0);
  icy_vector table = {.area = NULL, .free_indexes = NULL, .element_size = element_size};
  if(name != NULL){

    table.area = icy_mem_create(name);
    char name2[128] = {0};
    sprintf(name2, "%s.free", name);
    table.free_indexes = icy_mem_create(name2);
  }else{
    table.area = icy_mem_create3();
    table.free_indexes = icy_mem_create3();
  }
  
  if(table.free_indexes->size < sizeof(size_t)){
    icy_mem_realloc(table.free_indexes, sizeof(size_t));
    ((size_t *)table.free_indexes->ptr)[0] = 0;
  }

  if(table.area->size < element_size){
    icy_mem_realloc(table.area, element_size * 4);
    memset(table.area->ptr, 0, table.area->size);
    _icy_vector_alloc(&table);
  }
  ASSERT((table.area->size % table.element_size) == 0);
  ASSERT(icy_vector_count(&table) > 0);
  return iron_clone(&table, sizeof(table));
}


void icy_vector_destroy(icy_vector ** _table){
  icy_vector * table = *_table;
  *_table = NULL;
  icy_mem_free(table->area);
  icy_mem_free(table->free_indexes);
}

void * icy_vector_lookup(icy_vector * table, icy_index index){
  ASSERT(index.index < icy_vector_count(table));
  ASSERT(index.index > 0);
  return table->area->ptr + (4 + index.index) * table->element_size;
}

bool icy_vector_contains(icy_vector * table, icy_index index){
  if(index.index == 0)
    return false;
  if (index.index >= icy_vector_count(table))
    return false;
  size_t freecnt = _icy_vector_free_index_count(table);
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
void icy_vector_optimize(icy_vector * table){
  // the index table is optimized by sorting and removing excess elements
  //
  
  
  size_t free_cnt = _icy_vector_free_index_count(table); 
  if(table->free_indexes->ptr == NULL || free_cnt ==  0)
    return; // nothing to optimize.
    
  size_t * p = table->free_indexes->ptr;
  qsort(p + 1,free_cnt , sizeof(size_t), (void *) cmpfunc);
  size_t table_cnt = icy_vector_count(table);

  // remove all elements that are bigger than the table. note: how did these enter?
  while(p[free_cnt] >= table_cnt && free_cnt != 0){
    free_cnt -= 1;
  }

  // remove all elements that are freed from the end of the table
  while(table_cnt > 0 && p[free_cnt] == table_cnt - 1 && free_cnt != 0){
    table_cnt -= 1;
    free_cnt -= 1;
  }
  _icy_vector_free_index_count_set(table, free_cnt);
  icy_vector_count_set(table, table_cnt);

  size_t newsize = table_cnt * table->element_size + table->element_size * 5;
  icy_mem_realloc(table->area, newsize);
  size_t newsize2 = (1 + free_cnt) * sizeof(size_t);
  icy_mem_realloc(table->free_indexes, newsize2);
  icy_vector_check_sanity(table);
}
