
typedef struct{
  void * ptr;
  size_t size;
  char * name;
  int fd;
  bool only_32bit;
  bool persisted;
}icy_mem;

void icy_mem_set_data_directory(char * data_dir);
icy_mem * icy_mem_create(const char * name);
icy_mem * icy_mem_create2(const char * name, bool only32bit);
icy_mem * icy_mem_create3();


void icy_mem_realloc(icy_mem * area, size_t size);
void icy_mem_free(icy_mem * area);

extern bool icy_mem_emulate_memory_maps;
