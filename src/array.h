
void sort_indexed(i64 * ids, u64 count, u64 * out_indexes);
u64 count_uniques_sorted(i64 * items, u64 count);
void get_uniques(i64 * ids, u64 count, i64 * out_uniques);
bool are_sorted(i64 * arr, u64 count);

typedef bool (* selector)(const void *);
// Counts the items of data where selector returns true.
u64 count(void * data, size_t num, size_t element_size, selector selector_fcn);


void apply_arrayi(int * data, int cnt, int (* fcn)(int));
void apply_arrayii(int * data, int cnt, void (* fcn) (int, int));
void apply_arrayd(double * data, int cnt, double (* fcn) (double));
void apply_arraydi(double * data, int cnt, void (* fcn) (double,int));

// Sums the values.
i64 sum64(i64 * values, u64 count);

// List //
// adds an element to the array.
// not very efficient, but easy.
// requires dst to be allocated with alloc or NULL.
void list_add(void ** dst, size_t * cnt, const void * src, size_t item_size);
void list_insert(void ** dst, size_t * cnt, const void * src, size_t idx, size_t item_size);
void list_remove(void ** dst, size_t * cnt, size_t idx, size_t item_size);
void list_clean(void ** lst, size_t * cnt);

// The functions below are even more simple. They add / removes items from lists, but does not change count.
// Changing count is up to the user. This can be used for changing multiple arrays at the same time.
// Adds an item to the end of the array
#define list_push(array, cnt, item) array = ralloc(array,sizeof(array[0]) * ( cnt + 1)); array[cnt] = item;
#define list_push2(array, cnt, item) array = ralloc(array,sizeof(array[0]) * ( cnt + 1)); array[cnt] = item; cnt++;
// removes a specified index from the array. It does not reallocate.
#define list_remove2(array,cnt,index) memmove(&array[index],&array[index+1],(cnt - index - 1) * sizeof(array[0]));
// Returns true if all chars in str are whitespace.
bool all_whitespace(char * str);

void * array_find(void * array, size_t size, size_t elem_size, bool (* f)(void * item, void * userdata), void * userdata);
#define find1(array,size,f,userdata) array_find((void *) array, size, sizeof(array[0]), (bool (*)(void *, void *))f, userdata)

bool starts_with(const char *pre, const char *str);

// like memmem but with stride.
void * memmem2(void * haystack, size_t haystack_size, void * needle, size_t needle_size, size_t stride);
