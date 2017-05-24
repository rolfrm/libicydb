
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
      ASSERT(seq.index > 0);

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
      u32 idxes[200];
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
      //ASSERT(icy_vector_count(t) == 1);
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
    logd("ITERATION: %i\n", key);
    int_set_set(table, key);
    for(size_t i = 0; i < table->count + 1; i++){
      logd("key %i %i\n", i, table->key[i]);
    }
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
  
  logd("COUNT: %i\n", myTable->count);
  ASSERT(myTable->count == 40);
  bool test = true;
  for(size_t i = 0; i < myTable->count; i++){
    if(test){
      size_t key = i * 2;
      float x = sin(0.1 * key);
      float y = cos(0.1 * key);
      ASSERT(myTable->index[i + 1] == key);
      ASSERT(myTable->x[i + 1] == x);
      ASSERT(myTable->y[i + 1] == y);
    }else{
      logd("%i %f %f\n", myTable->index[i + 1], myTable->x[i + 1], myTable->y[i + 1]);
    }
  }
  size_t keys_to_remove[] = {8, 10, 12};
  size_t indexes_to_remove[array_count(keys_to_remove)];
  
  icy_table_finds((icy_table *) myTable, keys_to_remove, indexes_to_remove, array_count(keys_to_remove));
  icy_table_remove_indexes((icy_table *) myTable, indexes_to_remove, array_count(keys_to_remove));
  myTable->count = myTable->index_area->size / myTable->sizes[0] - 1;
  ASSERT(myTable->count == 40 - array_count(keys_to_remove));
  
  for(size_t i = 0; i < myTable->count; i++){
    if(test){
      ASSERT(myTable->index[i + 1] != 8 && myTable->index[i + 1] != 10 && myTable->index[i + 1] != 12);
      size_t _i = i * 2;
      if(_i >= 8)
	_i = i * 2 + 6;
      float x = sin(0.1 * _i);
      float y = cos(0.1 * _i);
      ASSERT(myTable->x[i + 1] == x);
      ASSERT(myTable->y[i + 1] == y);
    }
  }
  ASSERT(myTable->index[20] > 0);
  MyTableTest_set(myTable, 100, 4.5, 6.0);
  ASSERT(myTable->index[20] > 0);
  size_t key = 100, index = 0;
  MyTableTest_lookup(myTable, &key, &index, 1);
  TEST_ASSERT(index != 0);
  logd("%i %f %f\n", index, myTable->x[index], myTable->y[index]);
  TEST_ASSERT(myTable->x[index] == 4.5 && myTable->y[index] == 6.0);
  size_t idx = 20;
  float x;
  float y;
  ASSERT(MyTableTest_try_get(myTable, &idx, &x, &y));
  logd("%i %f %f\n", idx, x, y);
  idx = 21;
  ASSERT(MyTableTest_try_get(myTable, &idx, &x, &y) == false);
  MyTableTest_print(myTable);
  return TEST_SUCCESS;


}



int main(){

  TEST(icy_vector_test)
  TEST(int_lookup_test);
  return 0;
}
