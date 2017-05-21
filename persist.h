
typedef struct{
  void * ptr;
  size_t size;
  char * name;
  int fd;
  bool only_32bit;
  bool is_persisted;
}persisted_mem_area;

typedef persisted_mem_area mem_area;
void mem_area_set_data_directory(char * data_dir);
persisted_mem_area * create_mem_area(const char * name);
persisted_mem_area * create_mem_area2(const char * name, bool only32bit);
persisted_mem_area * create_non_persisted_mem_area();
#define mem_area_create(X) create_mem_area(X)
#define mem_area_create_non_persisted() create_non_persisted_mem_area()


void mem_area_realloc(persisted_mem_area * area, size_t size);
void mem_area_free(persisted_mem_area * area);
