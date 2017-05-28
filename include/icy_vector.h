// icy vector is a vector like data structurem where indexes can be looked up.
// it can also manage free indexes, so that items can be reused in the middle of the structure.
typedef struct{
  // the memory area stored the allocated elements.
  icy_mem * area;
  // the memory area storing the free indexes.
  icy_mem * free_indexes;
  // size of each element. This should not be changed.
  const size_t element_size;
}icy_vector;

// a vector index.
typedef struct{
  size_t index;
}icy_index;

// A sequence of allocated elements.
typedef struct{
  // The index of the first element.
  icy_index index;
  // the number of elements in the sequence.
  size_t count;
}icy_vector_sequence;

// creates a new icy_vector. name should be NULL if a non-persisted vector is wanted. Element size is the size of each allocated element in bytes.
icy_vector * icy_vector_create(const char * name, size_t element_size);

// Allocates a single element in the vector. Returns the index of it. the index can be looked up with icy_vector_lookup.
icy_index icy_vector_alloc(icy_vector * table);
// Allocates a sequence of elements. count is the number of elements that should be allocated.
icy_vector_sequence icy_vector_alloc_sequence(icy_vector * table, size_t count);
void * icy_vector_lookup_sequence(icy_vector * table, icy_vector_sequence seq);
void icy_vector_remove_sequence(icy_vector * table, icy_vector_sequence * seq);
void icy_vector_resize_sequence(icy_vector * table, icy_vector_sequence * seq, size_t new_count);
void * icy_vector_lookup(icy_vector * table, icy_index index);
void icy_vector_remove(icy_vector * table, icy_index index);
// Returns the number count of items in the vector. This number might not be the actually used number of elements, but rather the number of allocated slots.
size_t icy_vector_count(icy_vector * table);
// Returns a pointer to the first element in the vector and the number of elements.
void * icy_vector_all(icy_vector * table, size_t * out_cnt);
// clears the vector
void icy_vector_clear(icy_vector * table);
// returns true if the vector contains an index.
bool icy_vector_contains(icy_vector * table, icy_index index);
// optimizes the vector. This is function is dont automatically and is mostly exported for testing purposes.
void icy_vector_optimize(icy_vector * table);
// Destroys the icy vector. It does not remove the data on the disk
void icy_vector_destroy(icy_vector ** table);
