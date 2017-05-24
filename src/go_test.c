#include "int_set.h"
#include "int_set.c"
#include "log.h"
#include "test.h"
#include "mem.h"
#include <stdint.h>
#include "types.h"
#include "utils.h"
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


int main(){
  int_set * table = int_set_create("my-ints");
  int_set_clear(table);
  int_set_set(table, 5);
  int_set_set(table, 10);
  int_set_set(table, -10);
  int_set_set(table, -110);
  int_set_set(table, -11000);
  int_set_set(table, 11030);
  int_set_set(table, 11035);

  icy_vector_test();
  return 0;
}
