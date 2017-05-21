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
#include "persist.h"
#include "types.h"
#include "log.h"
#include "mem.h"
#include "array.h"
#include "utils.h"

persisted_mem_area * mem_areas = NULL;
u64 mem_area_cnt = 0;

u64 file_size(const char * path){
  int fd = open(path, O_RDWR | O_CREAT, 0666);
  u64 end = lseek(fd, 0, SEEK_END);
  close(fd);
  return end;
}

persisted_mem_area * get_mem_area_by_name(const char * name){
  for(u64 i = 0; i < mem_area_cnt; i++){
    if(strcmp(mem_areas[i].name, name) == 0)
      return mem_areas + i;
  }
  return NULL;
}

persisted_mem_area * get_mem_area_by_ptr(const void * ptr){
  for(u64 i = 0; i < mem_area_cnt; i++){
    if(mem_areas[i].ptr == ptr)
      return mem_areas + i;
  }
  return NULL;
}

persisted_mem_area * create_mem_area(const char * name){
  return create_mem_area2(name, false);
}
static const char * data_directory = "data";
void mem_area_set_data_directory(char * data_dir){
  data_directory = data_dir;
}

persisted_mem_area * create_mem_area2(const char * name, bool only_32bit){
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
  persisted_mem_area mema = {
    .ptr = ptr, .size = size,
    .name = (char *) name, .fd = fd,
    .only_32bit = only_32bit,
    .is_persisted = true
  };
  return IRON_CLONE(mema);
}

void mem_area_free(persisted_mem_area * area){
  if(area->is_persisted == false){
    if(area->ptr != NULL)
      dealloc(area->ptr);
  }else{
    munmap(area->ptr, area->size);
  }
  memset(area, 0, sizeof(*area));

}

persisted_mem_area * create_non_persisted_mem_area(){
  persisted_mem_area mema = {
    .ptr = NULL, .size = 0,
    .name = (char *)"", .fd = 0,
    .only_32bit = false
  };
  return iron_clone(&mema, sizeof(mema));
}

void * persist_alloc2(const char * name, size_t min_size, size_t * out_size){
  if(min_size == 0)
    min_size = 1;
  {
    persisted_mem_area * area = get_mem_area_by_name(name);
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
  //logd("new size: %i\n", size);
  lseek(fd, 0, SEEK_SET);
  void * ptr = mmap(NULL, size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0);
  ASSERT(ptr != NULL);
  persisted_mem_area mema = {.ptr = ptr, .size = size, .name = fmtstr("%s", name), .fd = fd};
  list_push2(mem_areas, mem_area_cnt, mema);
  *out_size = size;
  return ptr;
}


void * persist_alloc(const char * name, size_t min_size){
  size_t s = 0;
  return persist_alloc2(name, min_size, &s);
}

void mem_area_realloc(persisted_mem_area * area, u64 size){
  ASSERT(area != NULL);
  if(area->size == size) return;
  
  if(false == area->is_persisted){
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
