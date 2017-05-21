typedef struct _icy_table{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const unsigned int column_count;
  int (*cmp) (void * k1, void * k2);
  void * tail;
}icy_table;

void icy_table_init(icy_table * table, const char * table_name, unsigned int column_count, unsigned int * column_size, char ** column_name);
void icy_table_check_sanity(icy_table * table);
bool icy_table_keys_sorted(icy_table * table, void * keys, size_t cnt);
void icy_table_finds(icy_table * table, void * keys, size_t * indexes, size_t cnt);
void icy_table_inserts(icy_table * table, void ** values, size_t count);
void icy_table_clear(icy_table * table);
void icy_table_remove_indexes(icy_table * table, size_t * indexes, size_t index_count);
void icy_table_print(icy_table * table);
size_t icy_table_iter(icy_table * table, void * keys, size_t keycnt, void * out_keys, size_t * indexes, size_t cnt, size_t * idx);

void add_table_printer(bool (*printer)(void * ptr, const char * type));
