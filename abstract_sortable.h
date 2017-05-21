typedef struct _abstract_sorttable{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const unsigned int column_count;
  int (*cmp) (void * k1, void * k2);
  void * tail;
}abstract_sorttable;

void abstract_sorttable_init(abstract_sorttable * table, const char * table_name, unsigned int column_count, unsigned int * column_size, char ** column_name);
void abstract_sorttable_check_sanity(abstract_sorttable * table);
bool abstract_sorttable_keys_sorted(abstract_sorttable * table, void * keys, size_t cnt);
void abstract_sorttable_finds(abstract_sorttable * table, void * keys, size_t * indexes, size_t cnt);
void abstract_sorttable_inserts(abstract_sorttable * table, void ** values, size_t count);
void abstract_sorttable_clear(abstract_sorttable * table);
void abstract_sorttable_remove_indexes(abstract_sorttable * table, size_t * indexes, size_t index_count);
void abstract_sorttable_print(abstract_sorttable * table);
size_t abstract_sorttable_iter(abstract_sorttable * table, void * keys, size_t keycnt, void * out_keys, size_t * indexes, size_t cnt, size_t * idx);

void add_table_printer(bool (*printer)(void * ptr, const char * type));
