#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "icy_mem.h"
#include "types.h"
#include "log.h"
#include "mem.h"
#include "array.h"
#include "utils.h"

static icy_mem * icy_mems = NULL;
static u64 icy_mem_cnt = 0;

icy_mem * get_icy_mem_by_name(const char * name){
  for(u64 i = 0; i < icy_mem_cnt; i++){
    if(strcmp(icy_mems[i].name, name) == 0)
      return icy_mems + i;
  }
  return NULL;
}

icy_mem * get_icy_mem_by_ptr(const void * ptr){
  for(u64 i = 0; i < icy_mem_cnt; i++){
    if(icy_mems[i].ptr == ptr)
      return icy_mems + i;
  }
  return NULL;
}

icy_mem * icy_mem_create(const char * name){
  return icy_mem_create2(name, false);
}
static const char * data_directory = "data";
void icy_mem_set_data_directory(char * data_dir){
  data_directory = data_dir;
}

icy_mem * icy_mem_create2(const char * name, bool only_32bit){
  int fd = 0;
  u64 size = 0;
  if(name != NULL){
    ASSERT(name[0] != '/' && name[0] != 0);
    mkdir(data_directory, 0777);
    
    {
      char * pt = (char * )name;
      while(true){
	char * slash = strchr(pt, '/');
	if(slash == NULL) break;
	if(slash != NULL && slash[-1] != '\\'){
	  size_t size = slash - name;
	  char s[100];
	  strcpy(s, name);
	  s[size] = 0;
	  char buf[100];
	  sprintf(buf, "%s/%s", data_directory, s);
	  mkdir(buf, 0777);
	}
	pt = slash + 1;
	ASSERT(pt[0] != '/' && pt[0] != 0);
      }
    }
    const size_t min_size = 1;

    char path[100];
    sprintf(path, "%s/%s",data_directory, name);
    fd = open(path, O_RDWR | O_CREAT, 0666);
    ASSERT(fd >= 0);
    size = lseek(fd, 0, SEEK_END);
    if(size < min_size){
      lseek(fd, 0, SEEK_SET);
      ASSERT(0 == ftruncate(fd, min_size));
      size = lseek(fd, 0, SEEK_END);
    }
    lseek(fd, 0, SEEK_SET);
  }
  int flags = MAP_SHARED;
  if(only_32bit)
    flags |= MAP_32BIT;
  
  void * ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
  if(flags & MAP_32BIT){
    logd("32bit: %p\n", ptr);
  }
  ASSERT(ptr != NULL);
  icy_mem mema = {
    .ptr = ptr, .size = size,
    .name = (char *) name, .fd = fd,
    .only_32bit = only_32bit,
    .persisted = true
  };
  return IRON_CLONE(mema);
}

void icy_mem_free(icy_mem * area){
  if(area->persisted == false){
    if(area->ptr != NULL)
      dealloc(area->ptr);
  }else{
    munmap(area->ptr, area->size);
  }
  memset(area, 0, sizeof(*area));

}

icy_mem * icy_mem_create3(){
  icy_mem mema = {
    .ptr = NULL, .size = 0,
    .name = (char *)"", .fd = 0,
    .only_32bit = false
  };
  return iron_clone(&mema, sizeof(mema));
}

void * icy_mem_alloc2(const char * name, size_t min_size, size_t * out_size){
  if(min_size == 0)
    min_size = 1;
  {
    icy_mem * area = get_icy_mem_by_name(name);
    if(area != NULL){
      *out_size = area->size;
      return area->ptr;
    }
  }
  
  mkdir(data_directory, 0777);
  char path[100];
  sprintf(path, "%s/%s",data_directory, name);
  int fd = open(path, O_RDWR | O_CREAT, 0666);
  ASSERT(fd >= 0);
  u64 size = lseek(fd, 0, SEEK_END);
  if(size < min_size){
    lseek(fd, 0, SEEK_SET);
    ASSERT(0 == ftruncate(fd, min_size));
    size = lseek(fd, 0, SEEK_END);
  }
  lseek(fd, 0, SEEK_SET);
  void * ptr = mmap(NULL, size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0);
  ASSERT(ptr != NULL);
  icy_mem mema = {.ptr = ptr, .size = size, .name = fmtstr("%s", name), .fd = fd};
  list_push2(icy_mems, icy_mem_cnt, mema);
  *out_size = size;
  return ptr;
}


void * icy_mem_alloc(const char * name, size_t min_size){
  size_t s = 0;
  return icy_mem_alloc2(name, min_size, &s);
}

void icy_mem_realloc(icy_mem * area, u64 size){
  ASSERT(area != NULL);
  if(area->size == size) return;
  
  if(false == area->persisted){
    area->ptr = ralloc(area->ptr, area->size = size);
    return;
  }

  int flags = MREMAP_MAYMOVE;
  if(area->only_32bit)
    flags |= MAP_32BIT;
  ASSERT(size != 0);
  area->ptr = mremap(area->ptr, area->size, size, flags);
  ASSERT(0 == ftruncate(area->fd, size));
  
  area->size = size;
}
