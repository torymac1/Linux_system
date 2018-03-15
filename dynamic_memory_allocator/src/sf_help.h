#include <errno.h>
#include "sfmm.h"
#include <string.h>

#define WORD 2  //1 word = 2 bytes
#define ROW 8    //1 word = 8 bytes
#define PAGE 4096  // 1 page = 4096 bytes

extern void* start_of_heap;
extern void* end_of_heap;


void *insert_seg_free_list_helper(void *bp, int n);

void *insert_seg_free_list(void *bp);

void *remove_seg_free_list_helper(void *bp, int n);

void *remove_seg_free_list(void *bp);

void* find_fit(size_t size);

void *place(void *p, uint64_t size);

void *extend_heap();

void *coalescing_with_higher_address(void *ptr);

void *coalescing_with_lower_address(void *ptr);


/*
void test_sf_free_list();

void test_extand_heap();

void test_sf_malloc();

void test_sf_realloc();

*/
