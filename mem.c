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
#include "icydb_int.h"
static __thread allocator * _allocator = NULL;

ICY_HIDDEN void iron_set_allocator(allocator * alc){
  _allocator = alc;
}

ICY_HIDDEN allocator * iron_get_allocator(){
  return _allocator;
}

ICY_HIDDEN void with_allocator(allocator * alc, void (* cb)()){
  allocator * old_alloc = _allocator;
  _allocator = alc;
  cb();
  _allocator = old_alloc;
}

static void * _alloc(size_t size){
  if(_allocator == NULL) return malloc(size);
  return _allocator->alloc(_allocator, size);
}

ICY_HIDDEN void * alloc0(size_t size){
  void * ptr = _alloc(size);
  memset(ptr,0, size);
  return ptr;
}
ICY_HIDDEN void * alloc(size_t size){
  return _alloc(size);
}

ICY_HIDDEN void dealloc(void * ptr){
  if(_allocator == NULL){
    free(ptr);
  }else{
    _allocator->dealloc(_allocator, ptr);
  }
}

ICY_HIDDEN void * ralloc(void * ptr, size_t newsize){
  if(_allocator == NULL){
    return realloc(ptr, newsize); 
  }
  return _allocator->ralloc(_allocator, ptr,newsize);
}

ICY_HIDDEN void * iron_clone(const void * src, size_t s){
  void * out = alloc(s);
  memcpy(out, src, s);
  return out;
}

ICY_HIDDEN char * vfmtstr(const char * fmt, va_list args){
  va_list args2;
  va_copy(args2, args);
  size_t size = vsnprintf (NULL, 0, fmt, args2) + 1;
  va_end(args2);
  char * out = alloc(size);
  vsprintf (out, fmt, args);
  va_end(args);
  return out;
}

ICY_HIDDEN char * fmtstr(const char * fmt, ...){
  va_list args;
  va_start (args, fmt);
  return vfmtstr(fmt, args);
}

ICY_HIDDEN bool string_startswith(const char * target, const char * test){
  return strncmp(target, test, strlen(test)) == 0;
}

ICY_HIDDEN char * string_join(int cnt, const char * separator, char ** strings){
  if(cnt == 0) return fmtstr("");
  char * output = fmtstr("%s", strings[0]);
  for(int i = 1; i < cnt; i++){
    char * o = fmtstr("%s%s%s",output, separator, strings[i]);
    dealloc(output);
    output = o;
  }
  return output; 
}

ICY_HIDDEN int count_occurences(char * str, const char * pattern){
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

ICY_HIDDEN char ** string_split(char * str, const char * pattern, int * out_cnt){
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

ICY_HIDDEN void replace_inplace(char * out_buffer, const char * pattern, const char * insert){
  while(true){
    char * find = strstr(out_buffer, pattern);
    if(!find) return;
    int pattern_len = strlen(pattern), insert_len = strlen(insert);
    memmove(find + insert_len, find + pattern_len, strlen(find + pattern_len) + 1);
    memmove(find, insert, strlen(insert));
    out_buffer = find + insert_len;
  }
}
