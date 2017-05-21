#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "utils.h"
#include "mem.h"

#include "persist.h"
#include "index_table.h"
size_t index_table_capacity(index_table * table){
  return table->area->size / table->element_size - 4;
}

size_t index_table_count(index_table * table){
  return ((size_t *) table->area->ptr)[0];
}

void index_table_count_set(index_table * table, unsigned int newcount){
  ((size_t *) table->area->ptr)[0] = newcount;
}

size_t _index_table_free_index_count(index_table * table){
  return ((size_t *) table->free_indexes->ptr)[0];
}

void _index_table_free_index_count_set(index_table * table, size_t cnt){
  ((size_t *) table->free_indexes->ptr)[0] = cnt;
}
void * index_table_all(index_table * table, size_t * cnt){
  // return count -1 and a pointer from the placeholder element.
  *cnt = index_table_count(table) - 1;
  return table->area->ptr + table->element_size * 5;
}
void index_table_clear(index_table * table){
  index_table_count_set(table, 1);
  _index_table_free_index_count_set(table, 0);
}

size_t _index_table_alloc(index_table * table){
  size_t freeindexcnt = _index_table_free_index_count(table);
  if(freeindexcnt > 0){
    size_t idx = ((size_t *) table->free_indexes->ptr)[freeindexcnt];
    _index_table_free_index_count_set(table, freeindexcnt - 1);
    ASSERT(idx != 0);
    void * p = index_table_lookup(table, idx);
    memset(p, 0, table->element_size);
    return idx;
  }
  
  while(index_table_capacity(table) <= index_table_count(table)){
    size_t prevsize = table->area->size;
    ASSERT((prevsize % table->element_size) == 0);
    size_t newsize = MAX(prevsize * 2, 8 * table->element_size);
    mem_area_realloc(table->area, newsize);
    memset(table->area->ptr + prevsize, 0,  newsize - prevsize);
  }
  size_t idx = index_table_count(table);
  index_table_count_set(table,idx + 1);
  return idx;
}

size_t index_table_alloc(index_table * table){
  auto index = _index_table_alloc(table);
  ASSERT(index > 0);
  return index;
}


void index_table_remove(index_table * table, size_t index){
  ASSERT(index < index_table_count(table));
  ASSERT(index > 0);
  size_t cnt = _index_table_free_index_count(table);
  mem_area_realloc(table->free_indexes, table->free_indexes->size + sizeof(size_t));
  ((size_t *)table->free_indexes->ptr)[cnt + 1] = 0;
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), (cnt + 1) * sizeof(size_t), &index, sizeof(index)) == NULL);
  ((size_t *)table->free_indexes->ptr)[cnt + 1] = index;
  ((size_t *) table->free_indexes->ptr)[0] += 1;
}

void index_table_resize_sequence(index_table * table, index_table_sequence * seq,  size_t new_count){
  index_table_sequence nseq = {0};
  if(new_count == 0){
    index_table_remove_sequence(table, seq);
    *seq = nseq;
    return;
  }
  
  nseq = index_table_alloc_sequence(table, new_count);
  
  if(seq->index != 0 && seq->count != 0 ){
    if(new_count > 0){
      void * src = index_table_lookup_sequence(table, *seq);
      void * dst = index_table_lookup_sequence(table, nseq);
      memmove(dst, src, MIN(seq->count, nseq.count) * table->element_size);
    }
    index_table_remove_sequence(table, seq);
  }
  *seq = nseq;
}

index_table_sequence index_table_alloc_sequence(index_table * table, size_t  count){
  size_t freeindexcnt = _index_table_free_index_count(table);
  
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
	  _index_table_free_index_count_set(table, freeindexcnt - cnt);
	  size_t * ptr = table->free_indexes->ptr;
	  for(size_t j = i - cnt + 1; j < freeindexcnt - cnt; j++)
	    ptr[j + 1] = ptr[j + cnt + 1];
	  
	  ASSERT(start != 0);
	  void * p = index_table_lookup(table, start);
	  memset(p, 0, cnt * table->element_size);
	  return (index_table_sequence){.index = start, .count = cnt};
	}
      }
    }
  
  while(index_table_capacity(table) <= (index_table_count(table) + count)){
    size_t prevsize = table->area->size;
    ASSERT((prevsize % table->element_size) == 0);
    size_t newsize = MAX(prevsize * 2, 8 * table->element_size);
    mem_area_realloc(table->area, newsize);
    memset(table->area->ptr + prevsize, 0,  newsize - prevsize);
  }
  size_t idx = index_table_count(table);
  index_table_count_set(table,idx + count);
  return (index_table_sequence){.index = idx, .count = count};
}

void index_table_remove_sequence(index_table * table, index_table_sequence * seq){
  size_t cnt = _index_table_free_index_count(table);
  mem_area_realloc(table->free_indexes, table->free_indexes->size + seq->count * sizeof(size_t));
  //ASSERT(memmem(table->free_indexes->ptr + sizeof(size_t), cnt * sizeof(size_t), &index, sizeof(index)) == NULL);
  for(size_t i = 0; i < seq->count; i++)
    ((size_t *)table->free_indexes->ptr)[cnt + i + 1] = seq->index + i;
  ((size_t *) table->free_indexes->ptr)[0] += seq->count;
  memset(seq, 0, sizeof(*seq));
  index_table_optimize(table);
}

void * index_table_lookup_sequence(index_table * table, index_table_sequence seq){
  if(seq.index == 0)
    return NULL;
  return index_table_lookup(table, seq.index);
}


index_table * index_table_create(const char * name, size_t element_size){
  ASSERT(element_size > 0);
  index_table table = {0};
  table.element_size = element_size;
  if(name != NULL){

    table.area = mem_area_create(name);
    char name2[128] = {0};
    sprintf(name2, "%s.free", name);
    table.free_indexes = mem_area_create(name2);
  }else{
    table.area = mem_area_create_non_persisted();
    table.free_indexes = mem_area_create_non_persisted();
  }
  
  if(table.free_indexes->size < sizeof(size_t)){
    mem_area_realloc(table.free_indexes, sizeof(size_t));
    ((size_t *)table.free_indexes->ptr)[0] = 0;
  }

  if(table.area->size < element_size){
    mem_area_realloc(table.area, element_size * 4);
    memset(table.area->ptr, 0, table.area->size);
    _index_table_alloc(&table);
  }
  ASSERT((table.area->size % table.element_size) == 0);
  ASSERT(index_table_count(&table) > 0);
  return iron_clone(&table, sizeof(table));
}


void index_table_destroy(index_table ** _table){
  index_table * table = *_table;
  *_table = NULL;
  mem_area_free(table->area);
  mem_area_free(table->free_indexes);
}

void * index_table_lookup(index_table * table, size_t index){
  ASSERT(index < index_table_count(table));
  ASSERT(index > 0);
  return table->area->ptr + (4 + index) * table->element_size;
}

bool index_table_contains(index_table * table, size_t index){
  if(index == 0)
    return false;
  if (index >= index_table_count(table))
    return false;
  size_t freecnt = _index_table_free_index_count(table);
  size_t * start = table->free_indexes->ptr + sizeof(size_t);
  for(size_t i = 0; i < freecnt; i++){
    if(start[i] == index)
      return false;
  }
  return true;
}

void index_table_optimize(index_table * table){
  // the index table is optimized by sorting and removing excess elements
  //
  int cmpfunc (const size_t * a, const size_t * b)
  {
    return ( *(int*)a - *(int*)b );
  }
  
  size_t free_cnt = _index_table_free_index_count(table); 
  if(table->free_indexes->ptr == NULL || free_cnt ==  0)
    return; // nothing to optimize.
    
  size_t * p = table->free_indexes->ptr;
  qsort(p + 1,free_cnt , sizeof(size_t), (void *) cmpfunc);
  size_t table_cnt = index_table_count(table);

  // remove all elements that are bigger than the table. note: how did these enter?
  while(p[free_cnt] >= table_cnt && free_cnt != 0){
    free_cnt -= 1;
  }

  // remove all elements that are freed from the end of the table
  while(table_cnt > 0 && p[free_cnt] == table_cnt - 1 && free_cnt != 0){
    table_cnt -= 1;
    free_cnt -= 1;
  }
  _index_table_free_index_count_set(table, free_cnt);
  index_table_count_set(table, table_cnt);
  size_t newsize = table_cnt * table->element_size + table->element_size * 5;
  mem_area_realloc(table->area, newsize);
  size_t newsize2 = free_cnt * table->element_size + sizeof(size_t);
  mem_area_realloc(table->free_indexes, newsize2);
}
