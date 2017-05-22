typedef struct{
  icy_mem * area;
  icy_mem * free_indexes;
  size_t element_size;
}icy_vector;

typedef struct{
  size_t index;
  size_t count;

}icy_vector_sequence;

icy_vector * icy_vector_create(const char * name, size_t element_size);
size_t icy_vector_alloc(icy_vector * table);
icy_vector_sequence icy_vector_alloc_sequence(icy_vector * table, size_t count);
void * icy_vector_lookup_sequence(icy_vector * table, icy_vector_sequence seq);
void icy_vector_remove_sequence(icy_vector * table, icy_vector_sequence * seq);
void icy_vector_resize_sequence(icy_vector * table, icy_vector_sequence * seq, size_t new_count);
void * icy_vector_lookup(icy_vector * table, size_t index);
void icy_vector_remove(icy_vector * table, size_t index);
size_t icy_vector_count(icy_vector * table);
void * icy_vector_all(icy_vector * table, size_t * cnt);
void icy_vector_clear(icy_vector * table);
bool icy_vector_contains(icy_vector * table, size_t index);
void icy_vector_optimize(icy_vector * table);
void icy_vector_destroy(icy_vector ** table);
