
#include "int_set.h"
#include "int_set.c"
#include "MyTableTest.h"
#include "MyTableTest.c"
#include "log.h"
#include "test.h"
#include "mem.h"
#include <stdint.h>
#include "types.h"
#include "utils.h"
#include "math.h"

bool icy_vector_test(){
  {// first test.

    { // this was a bug once:
      icy_vector * t = icy_vector_create("__icy_vector_test__", sizeof(u32));
      icy_vector_clear(t);
      icy_vector_optimize(t);
      icy_vector_sequence seq = icy_vector_alloc_sequence(t, 10);
      icy_vector_sequence seq2 = icy_vector_alloc_sequence(t, 10);
      icy_vector_remove_sequence(t, &seq2); // crash happened here.
      icy_vector_remove_sequence(t, &seq); 
      UNUSED(seq);
      icy_vector_clear(t);
      icy_vector_destroy(&t);
    }

    for(int k = 1; k < 4;k++){
      icy_vector * t = icy_vector_create("__icy_vector_test__", sizeof(u32));
      icy_vector_clear(t);
      icy_vector_optimize(t);
      icy_vector_sequence seq = icy_vector_alloc_sequence(t, 10);
      icy_vector_sequence seq2 = icy_vector_alloc_sequence(t, 10);
      icy_vector_remove_sequence(t, &seq2);
      icy_vector_remove_sequence(t,&seq);
      seq = icy_vector_alloc_sequence(t, 20);
      seq2 = icy_vector_alloc_sequence(t, 20);
      icy_vector_remove_sequence(t,&seq2);
      icy_vector_remove_sequence(t,&seq);
      seq = icy_vector_alloc_sequence(t, 20);
      seq2 = icy_vector_alloc_sequence(t, 20);
    
      icy_vector_resize_sequence(t, &seq, 22);
      icy_vector_resize_sequence(t, &seq2, 22);
      icy_vector_resize_sequence(t, &seq, 21);
      icy_vector_resize_sequence(t, &seq2, 21);
    
      icy_vector_resize_sequence(t, &seq, 9);
      icy_vector_remove_sequence(t, &seq);      
      icy_vector_optimize(t);
      seq = icy_vector_alloc_sequence(t, 20);
      ASSERT(seq.count == 20);
      ASSERT(seq.index.index > 0);

      for(u32 i = 1; i < 5; i++){
	u32 * pts = icy_vector_lookup_sequence(t, seq);
	u32 * pts2 = icy_vector_lookup_sequence(t, seq2);
	for(u32 i = 0; i < 20; i++){
	  pts[i] = i;
	  pts2[i] = i + 50;
	}
	
	icy_vector_resize_sequence(t, &seq, 21 + i * 50);
	icy_vector_resize_sequence(t, &seq2, 21 + i * 50);
	icy_vector_resize_sequence(t, &seq, 21);
	icy_vector_resize_sequence(t, &seq2, 21);

	pts = icy_vector_lookup_sequence(t, seq);
	pts2 = icy_vector_lookup_sequence(t, seq2);
	for(u32 i = 0; i < 20; i++){
	  ASSERT(pts[i] == i);
	  ASSERT(pts2[i] == i + 50);
	}
      }
      
      {
	icy_vector_sequence seq3 = icy_vector_alloc_sequence(t, 10);
	icy_vector_sequence seq4 = icy_vector_alloc_sequence(t, 40);
	icy_vector_sequence seq2 = icy_vector_alloc_sequence(t, 20);
	//ASSERT(seq2.index == i1);
	icy_vector_remove_sequence(t, &seq2);
	icy_vector_remove_sequence(t, &seq3);
	icy_vector_remove_sequence(t, &seq4); 
	icy_vector_optimize(t);
	icy_index idxes[200];
	for(u32 j = 3; j < 20; j++){
	  for(u32 i = 0; i < j * 10; i++)
	    idxes[i] = icy_vector_alloc(t);
	  for(u32 i = 0; i < j * 10; i++)
	    ((u32 *) icy_vector_lookup(t, idxes[i]))[0] = 0xFFDDCCAA * k;
	  for(u32 i = 0; i < j * 10; i++)
	    ASSERT(((u32 *) icy_vector_lookup(t, idxes[i]))[0] == 0xFFDDCCAA * k);
	  for(u32 i = 0; i < j * 10; i++)
	    icy_vector_remove(t, idxes[i]);
	}
	icy_vector_optimize(t);
      }
      
      icy_vector_clear(t);
      icy_vector_destroy(&t);
    }
  }
  return TEST_SUCCESS;
}

bool int_lookup_test(){
    int_set * table = int_set_create(NULL);
  for(int key = 100; key < 200; key+=2){
    int_set_set(table, key);
    ASSERT(int_set_try_get(table, &key));
  }
  
  for(int key = 100; key < 200; key++){
    if(key%2 == 0){
      ASSERT(int_set_try_get(table, &key));
    }else{
      ASSERT(false == int_set_try_get(table, &key));
    }
  }

  MyTableTest * myTable = MyTableTest_create("MyTable");
  MyTableTest_clear(myTable);
  for(size_t j = 0; j < 2; j++){
    size_t keys[40];
    float xs[40];
    float ys[40];
    
    for(size_t i = 0; i < 40; i++){
      size_t key = i * 2;

      float x = sin(0.1 * key);
      float y = cos(0.1 * key);
      keys[i] = key;
      xs[i] = x;
      ys[i] = y;
    }
    MyTableTest_insert(myTable, keys, xs, ys, 40);
  }
  
  ASSERT(myTable->count == 40);
  for(size_t i = 0; i < myTable->count; i++){
    size_t key = i * 2;
    float x = sin(0.1 * key);
    float y = cos(0.1 * key);
    ASSERT(myTable->index[i + 1] == key);
    ASSERT(myTable->x[i + 1] == x);
    ASSERT(myTable->y[i + 1] == y);
  }
  size_t keys_to_remove[] = {8, 10, 12};
  size_t indexes_to_remove[array_count(keys_to_remove)];
  
  icy_table_finds((icy_table *) myTable, keys_to_remove, indexes_to_remove, array_count(keys_to_remove));
  icy_table_remove_indexes((icy_table *) myTable, indexes_to_remove, array_count(keys_to_remove));
  myTable->count = myTable->index_area->size / myTable->sizes[0] - 1;
  ASSERT(myTable->count == 40 - array_count(keys_to_remove));
  
  for(size_t i = 0; i < myTable->count; i++){
    ASSERT(myTable->index[i + 1] != 8 && myTable->index[i + 1] != 10 && myTable->index[i + 1] != 12);
    size_t _i = i * 2;
    if(_i >= 8)
      _i = i * 2 + 6;
    float x = sin(0.1 * _i);
    float y = cos(0.1 * _i);
    ASSERT(myTable->x[i + 1] == x);
    ASSERT(myTable->y[i + 1] == y);
  }
  ASSERT(myTable->index[20] > 0);
  MyTableTest_set(myTable, 100, 4.5, 6.0);
  ASSERT(myTable->index[20] > 0);
  size_t key = 100, index = 0;
  MyTableTest_lookup(myTable, &key, &index, 1);
  TEST_ASSERT(index != 0);
  TEST_ASSERT(myTable->x[index] == 4.5 && myTable->y[index] == 6.0);
  size_t idx = 20;
  float x;
  float y;
  ASSERT(MyTableTest_try_get(myTable, &idx, &x, &y));
  idx = 21;
  ASSERT(MyTableTest_try_get(myTable, &idx, &x, &y) == false);
  MyTableTest_print(myTable);
  return TEST_SUCCESS;
}

typedef struct{
  char ** column_names;
  char ** column_types;
  size_t * count;
  size_t * capacity;
  size_t * free_index_count;
  const size_t column_count;
  icy_mem * free_indexes;
  icy_mem * header;
  size_t column_sizes[2];
  int * a;
  int * b;
  icy_mem * mem_a;
  icy_mem * mem_b;
  
}icy_vector_abs_test_struct;

bool icy_vector_abs_test(){
  logd("ICY VECTOR ABS TEST\n");

  icy_vector_abs_test_struct * testtable = alloc0(sizeof(icy_vector_abs_test_struct));
  const char * names[] =  {"a", "b"};
  const char * types[] =  {"int", "int"};
  
  testtable->column_names = (char **)names;
  testtable->column_types = (char **)types;
  ((size_t *)&testtable->column_count)[0] = 2;
  testtable->column_sizes[0] = sizeof(int);
  testtable->column_sizes[1] = sizeof(int);
  icy_vector_abs_init((icy_vector_abs *) testtable, "test_abs");
  icy_index i1 = icy_vector_abs_alloc((icy_vector_abs *) testtable);
  icy_index i2 = icy_vector_abs_alloc((icy_vector_abs *) testtable);
  logd("%i %i %i %i %i %i\n", i1.index, i2.index, testtable->a, testtable->b, testtable->capacity[0], testtable->count[0]);
  testtable->a[i1.index] = 5;
  testtable->b[i1.index] = 10;
  testtable->a[i2.index] = 10;
  testtable->b[i2.index] = 5;

  icy_vector_abs_sequence seq = icy_vector_abs_alloc_sequence((icy_vector_abs *)testtable, 10);
  for(size_t i = 0; i < seq.count; i++){
    testtable->a[seq.index.index + i] = 12 + i;
    testtable->b[seq.index.index + i] = 101 - i;
  }
  
  for(size_t i = 0; i < testtable->count[0]; i++){
    logd("%i %i\n", testtable->a[i], testtable->b[i]);
  }
  size_t init_cnt = *testtable->count;
  size_t init_cap = *testtable->capacity;
  icy_vector_abs_remove((icy_vector_abs *)testtable, i1);
  icy_vector_abs_remove((icy_vector_abs *)testtable, i2);
  ASSERT(init_cnt == *testtable->count);
  init_cnt = *testtable->count;
  icy_vector_abs_remove_sequence((icy_vector_abs *)testtable, &seq);
  ASSERT(init_cnt > *testtable->count);
  ASSERT(init_cap > *testtable->capacity);
  logd("End: %i %i\n", *testtable->count, *testtable->capacity);
  

  return TEST_SUCCESS;
}
/*
#include "myvec.h"
#include "myvec.c"

bool myvec_test(){
  logd("MYVEC TEST\n");
  myvec * tab = myvec_create("test_table");
  myvec_index idx = myvec_alloc(tab);
  ASSERT(*tab->count > 0);

  logd("%i %i\n", *tab->count, *tab->capacity);

  myvec_indexes indexes = myvec_alloc_sequence(tab, 100);
  for(u32 i = 0; i < indexes.count; i++){
    tab->col1[i] = i * 2;
    tab->col2[i] = i * 4;
  }
  ASSERT(*tab->count != 0);
  
  myvec_remove_sequence(tab, &indexes);
  // remove sequence has built in optimize
  ASSERT(*tab->count > 0);

  
  myvec_remove(tab, idx);
  
  myvec_optimize(tab);
  ASSERT(*tab->count == 0);
  return TEST_SUCCESS;
  }*/


int main(){

  TEST(icy_vector_test);
  TEST(int_lookup_test);
  TEST(icy_vector_abs_test);
  //TEST(myvec_test);
  return 0;
}
