#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include <stdarg.h>
#include "types.h"
#include "array.h"
#include "mem.h"
#include "icydb_int.h"
ICY_HIDDEN i64 sum64(i64 * data, u64 len){
  i64 a = 0;
  for(u64 i = 0; i < len; i++)
    a += data[i];
  return a;
}

ICY_HIDDEN u64 count(void * data, size_t num, size_t size, selector selector_fcn){
  u64 cnt = 0;
  for(size_t i = 0; i < num; i++){
    size_t offset = i * size;
    cnt += selector_fcn(data + offset) ? 1 : 0;
  }
  return cnt;
}

ICY_HIDDEN void sort_indexed(i64 * ids, u64 count, u64 * out_indexes){
  int comp (const void * elem1, const void * elem2) 
  {
    u64 i1 = *((u64*)elem1);
    u64 i2 = *((u64*)elem2);
    if (ids[i1] > ids[i2]) return  1;
    if (ids[i1] < ids[i2]) return -1;
    return 0;
  }

  for(u64 i = 0; i < count;i++){
    out_indexes[i] = i;
  }
  qsort(out_indexes,count,sizeof(i64),&comp);
}

ICY_HIDDEN u64 count_uniques_sorted(i64 * ids, u64 count){
  if(count == 0) return 0;
  u64 out = 1;
  i64 current = ids[0];
  for(u64 i = 0; i < count; i++){
    if(ids[i] != current){
      current = ids[i];
      out++;
    }
  }
  return out;
}

// Assumes there is room in out_uniques.
ICY_HIDDEN void get_uniques(i64 * ids, u64 count, i64 * out_uniques){
  if(count == 0) return;
  i64 current = ids[0];
  *out_uniques++ = current;
  for(u64 i = 0; i < count; i++){
    if(ids[i] != current){
      current = ids[i];
      *out_uniques++ = current;
    }
  }
}

ICY_HIDDEN bool are_sorted(i64 * arr, u64 count){
  for(u64 i = 1; i < count ;i++)
    if(arr[i - 1] > arr[i])
      return false;
  return true;
}

ICY_HIDDEN void apply_arrayii(int * data, int cnt, void (* fcn) (int, int)){
  for(int i = 0; i < cnt; i++)
    fcn(data[i],i);
}

ICY_HIDDEN void apply_arrayi(int * data, int cnt, int (* fcn) (int)){
  for(int i = 0; i < cnt; i++)
    data[i] = fcn(data[i]);
}

ICY_HIDDEN void apply_arrayd(double * data, int cnt, double (* fcn) (double)){
  for(int i = 0; i < cnt; i++)
    data[i] = fcn(data[i]);
}

ICY_HIDDEN void apply_arraydi(double * data, int cnt, void (* fcn) (double,int)){
  for(int i = 0; i < cnt; i++)
    fcn(data[i],i);
}


ICY_HIDDEN void list_add(void ** dst, size_t * cnt, const void * src, size_t item_size){
  size_t next_size = ++(*cnt);
  void * ptr = *dst;
  ptr =  ralloc(ptr, next_size * item_size);
  *dst = ptr;
  memcpy(ptr + (next_size - 1) * item_size, src, item_size);
}

ICY_HIDDEN void list_remove(void ** lst, size_t * cnt, size_t idx, size_t item_size){
  size_t offset = idx * item_size;
  size_t rest_bytes = (*cnt - idx - 1) * item_size;
  memcpy(*lst + offset, *lst + offset + item_size, rest_bytes);
  *cnt -= 1;
  *lst = ralloc(*lst, *cnt * item_size);
}


ICY_HIDDEN void list_clean(void ** lst, size_t * cnt){
  dealloc(*lst);
  *lst = NULL;
  *cnt = 0;
}

#include <ctype.h>
ICY_HIDDEN bool all_whitespace(char * str){
  while(*str != 0)
    if(isspace(*str) == false)
      return false;
  return true;
}

ICY_HIDDEN void * array_find(void * array, size_t size, size_t elem_size, bool (* f)(void * item, void * userdata), void * userdata){
  for(size_t i = 0; i < size; i++){
    if(f(array, userdata))
      return array;
    array += elem_size;
  }
  return NULL;
}

ICY_HIDDEN bool starts_with(const char *pre, const char *str)
{
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}


ICY_HIDDEN void * memmem2(void * haystack, size_t haystack_size, void * needle, size_t needle_size, size_t stride){
  void * end = haystack + haystack_size;
  for(;haystack < end; haystack += stride)
    if(memcmp(haystack, needle, needle_size) == 0)
      return haystack;
  return NULL;
}
