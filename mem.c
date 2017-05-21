#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio.h"
#include "types.h"
#include "mem.h"
#include "utils.h"
#include "math.h"
#include "log.h"
#include "array.h"
__thread allocator * _allocator = NULL;

void iron_set_allocator(allocator * alc){
  _allocator = alc;
}

allocator * iron_get_allocator(){
  return _allocator;
}

void with_allocator(allocator * alc, void (* cb)()){
  allocator * old_alloc = _allocator;
  _allocator = alc;
  cb();
  _allocator = old_alloc;
}

void * _alloc(size_t size){
  if(_allocator == NULL) return malloc(size);
  return _allocator->alloc(_allocator, size);
}

void * alloc0(size_t size){
  void * ptr = _alloc(size);
  memset(ptr,0, size);
  return ptr;
}
void * alloc(size_t size){
  return _alloc(size);
}

void dealloc(void * ptr){
  if(_allocator == NULL){
    free(ptr);
  }else{
    _allocator->dealloc(_allocator, ptr);
  }
}

void * ralloc(void * ptr, size_t newsize){
  if(_allocator == NULL){
    return realloc(ptr, newsize); 
  }
  return _allocator->ralloc(_allocator, ptr,newsize);
}

void * iron_clone(const void * src, size_t s){
  void * out = alloc(s);
  memcpy(out, src, s);
  return out;
}

struct _block_chunk;
typedef struct _block_chunk block_chunk;

struct _block_chunk{
  size_t size;
  size_t size_left;
  void * block_front;
  void * block_start;
  block_chunk * last;
  void ** ptrs;
  size_t * ptrs_size;
  size_t cnt; 
  void ** free_ptrs;
  size_t * free_ptrs_size;
  size_t free_cnt;
  
};

void * block_take_free(size_t s, block_chunk * bc){
  while(bc != NULL){
    for(size_t i = 0; i < bc->free_cnt; i++)
      if(bc->free_ptrs_size[i] <= s){
	void * freeptr = bc->free_ptrs[i];
	with_allocator(NULL, lambda(void, (){
	      list_remove((void **) &bc->free_ptrs, &bc->free_cnt, i, sizeof(void *));
	      bc->free_cnt++;
	      list_remove((void **) &bc->free_ptrs_size, &bc->free_cnt, i, sizeof(size_t));
	    }));
	return freeptr;

      }
    bc = bc->last;
  }
  return NULL;
}

void * block_alloc(allocator * self, size_t size){
  block_chunk * balc = self->user_data;
  void * freeptr = block_take_free(size, balc);
  if(freeptr != NULL)
    return freeptr;
  if(balc == NULL){
    block_chunk * newchunk = calloc(1, sizeof(block_chunk));
    size_t start_size = size == 0 ? 128 : size * 8;
    newchunk->block_start = malloc(start_size);
    newchunk->block_front = newchunk->block_start;
    newchunk->size = start_size;
    newchunk->size_left = start_size;
    balc = newchunk;
    self->user_data = balc;
  }


  if(balc->size_left < size){
    block_chunk * newchunk = calloc(1, sizeof(block_chunk));
    size_t start_size = balc->size * 2;
    while(start_size < size) start_size *= 2;
    
    newchunk->block_start = malloc(start_size);
    newchunk->block_front = newchunk->block_start;
    newchunk->size = start_size;
    newchunk->size_left = start_size;
    newchunk->last = balc;
    balc = newchunk;
    self->user_data = balc;
  }

  void * ret = balc->block_front;
  balc->size_left -= size;
  balc->block_front += size;
  with_allocator(NULL, lambda(void, (){
	list_add((void **) &balc->ptrs, &balc->cnt,&ret, sizeof(void *));
	balc->cnt--;
	list_add((void **) &balc->ptrs_size, &balc->cnt,&size, sizeof(size_t));
      }));
  return ret;
}

void block_dealloc(allocator * self, void * ptr){
  block_chunk * balc = self->user_data;
  while(balc != NULL && balc->block_start > ptr)
    balc = balc->last;
  if(balc == NULL) {
    ERROR("No pointer");
    return;
  }

  size_t size = 0;
  for(size_t i = 0; i < balc->cnt; i++){
    if(balc->ptrs[i] == ptr){
      size = balc->ptrs_size[i];
      break;
    }
  }
  with_allocator(NULL, lambda(void, (){
	list_add((void **) &balc->free_ptrs, &balc->free_cnt, &ptr, sizeof(void *));
	balc->free_cnt--;
	list_add((void **) &balc->free_ptrs_size, &balc->free_cnt, &size, sizeof(size_t));
      }));
}

void * block_ralloc(allocator * self, void * ptr, size_t s){
  block_chunk * front = self->user_data;
  block_chunk * balc = front;
  if(balc == NULL || ptr == NULL){
    ASSERT(ptr == NULL);
    return block_alloc(self, s);
  }
  while(balc != NULL && balc->block_start > ptr)
    balc = balc->last;
  
  ASSERT(balc != NULL);
  for(size_t i = 0; i < balc->cnt; i++){
    logd("%p == %p  | %p\n", ptr, balc->ptrs[i], balc->block_start);
    if(balc->ptrs[i] == ptr){
      size_t sn = balc->ptrs_size[i];
      if(sn >= s){
	return ptr;
      }else{
	if(i < balc->cnt - 1){
	  void * nextptr = balc->ptrs[i];
	  for(size_t j = 0; j < balc->free_cnt; j++){
	    if(balc->free_ptrs[j] == nextptr){
	      //absorb this ptr
	      with_allocator(NULL, lambda(void, (){
		    balc->ptrs_size[i] += balc->free_ptrs_size[j];
		    list_remove((void **) &balc->free_ptrs_size, &balc->free_cnt, j, sizeof(size_t));
		    balc->free_cnt++;
		    list_remove((void **) &balc->free_ptrs, &balc->free_cnt, j, sizeof(void *));
		    list_remove((void **) &balc->ptrs_size, &balc->cnt, j, sizeof(size_t));
		    balc->cnt++;
		    list_remove((void **) &balc->ptrs, &balc->cnt, j, sizeof(void *));
		  }));
	      if(balc->ptrs_size[i] <= s){
		return balc->ptrs[i];
	      }else{
		// sigh..
		return block_ralloc(self, ptr, s);
	      }
	    }
	  }
	}
	else if(balc->size_left + sn <= s){
	  balc->ptrs_size[i] = s;
	  return ptr;
	}
      }
      void * nptr = block_alloc(self, s);
      memcpy(nptr, ptr, sn);
      block_dealloc(self, ptr);
      return nptr;
    }
  }
  logd("balc-cnt: %i %p\n", balc->cnt, ptr);
  ERROR("Should never happen!");
  ASSERT(false);
  return ptr;
}

allocator * block_allocator_make(){
  allocator * alc = malloc(sizeof(allocator));
  alc->alloc = block_alloc;
  alc->dealloc = block_dealloc;
  alc->ralloc = block_ralloc;
  alc->user_data = NULL;
  return alc;
}

void block_allocator_release(allocator * block_allocator){
  block_chunk * balc = block_allocator->user_data;
  while(balc != NULL){
    block_chunk * next = balc->last;
    if(balc->ptrs != NULL)
      list_clean((void **) &balc->ptrs, &balc->cnt);
    free(balc->block_start);
    free(balc);
    balc = next;
  }
}

void * trace_alloc(allocator * self, size_t size){
  u64 current_size = (u64) self->user_data;
  current_size += 1;
  self->user_data = (void *) current_size;
  return malloc(size);
}

void trace_dealloc(allocator * self,void * ptr){
  self->user_data--;
  free(ptr);
}

void * trace_ralloc(allocator * self, void * ptr, size_t s){
  if(ptr == NULL) return trace_alloc(self, s);
  return realloc(ptr,s);
}


allocator * trace_allocator_make(){
  allocator * alc = alloc(sizeof(allocator));
  alc->alloc = trace_alloc;
  alc->dealloc = trace_dealloc;
  alc->ralloc = trace_ralloc;
  alc->user_data = 0;
  return alc;
}

void trace_allocator_release(allocator * aloc){
  dealloc(aloc);
}

size_t trace_allocator_allocated_pointers(allocator * trace_allocator){
  return (size_t) trace_allocator->user_data;
}

char * vfmtstr(const char * fmt, va_list args){
  va_list args2;
  va_copy(args2, args);
  size_t size = vsnprintf (NULL, 0, fmt, args2) + 1;
  va_end(args2);
  char * out = alloc(size);
  vsprintf (out, fmt, args);
  va_end(args);
  return out;
}

char * fmtstr(const char * fmt, ...){
  va_list args;
  va_start (args, fmt);
  return vfmtstr(fmt, args);
}

bool string_startswith(const char * target, const char * test){
  return strncmp(target, test, strlen(test)) == 0;
}

char * string_join(int cnt, const char * separator, char ** strings){
  if(cnt == 0) return fmtstr("");
  char * output = fmtstr("%s", strings[0]);
  for(int i = 1; i < cnt; i++){
    char * o = fmtstr("%s%s%s",output, separator, strings[i]);
    dealloc(output);
    output = o;
  }
  return output; 
}

int count_occurences(char * str, const char * pattern){
  int i = 0;
  while(str){
    str = strstr(str, pattern);
    if(str == NULL)
      break;
    str += strlen(pattern);
    i++;
  }
  return i;
}

char ** string_split(char * str, const char * pattern, int * out_cnt){
  int outcnt = count_occurences(str, pattern);
  char * outputs [outcnt + 1];
  int i = 0;
  bool doBreak = false;
  while(!doBreak){
    char * str2 = strstr(str, pattern);
    if(str2 == NULL){
      str2 = str + strlen(str);
      doBreak = true;
    }
    int cnt = str2 - str;
    char buf[cnt + 1];
    memmove(buf, str, cnt);
    buf[cnt] = 0;
    outputs[i] = fmtstr("%s", buf);
    i++;
    str = str2 + strlen(pattern);
  }
  *out_cnt = i;
  return iron_clone(outputs, sizeof(outputs));
}

void replace_inplace(char * out_buffer, const char * pattern, const char * insert){
  while(true){
    char * find = strstr(out_buffer, pattern);
    if(!find) return;
    int pattern_len = strlen(pattern), insert_len = strlen(insert);
    memmove(find + insert_len, find + pattern_len, strlen(find + pattern_len) + 1);
    memmove(find, insert, strlen(insert));
    out_buffer = find + insert_len;
  }
}
