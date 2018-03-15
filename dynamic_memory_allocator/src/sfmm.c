/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

#include "sf_help.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

void* start_of_heap;
void* end_of_heap;

int sf_errno = 0;

int page = 0;

void write_block(void* ptr, uint64_t block_size, uint64_t requested_size, uint64_t allocated, uint64_t padded){
    //printf("write_block to address %p for requested_size %lu\n", ptr, requested_size);
    sf_header *header = ptr;
    sf_footer *footer = ptr + block_size - SF_FOOTER_SIZE/8;   //SF_FOOTER_SIZE/8 convert bits to bytes

    header -> allocated = allocated;
    header -> padded = padded;
    header -> block_size = block_size >> 4;

    footer -> allocated = allocated;
    footer -> padded = padded;
    footer -> block_size = block_size >> 4;
    footer -> requested_size = requested_size;
}

void *insert_seg_free_list_helper(void *bp, int n){
    sf_free_header *header = bp;
    if (seg_free_list[n].head == NULL){         //empty linklist seg_free_list, the head = ptr
        seg_free_list[n].head = header;
        header -> next = NULL;
        header -> prev = NULL;
    }

    else{                                       //not empty, then insert ptr to head of list
        header -> next = seg_free_list[n].head;
        header -> prev = NULL;
        seg_free_list[n].head -> prev = header;
        seg_free_list[n].head = header;
    }
    return NULL;
}

void *insert_seg_free_list(void *bp){
    sf_free_header *ptr = bp;
    uint64_t asize = ptr -> header.block_size << 4;
    if (LIST_1_MIN <= asize && asize <= LIST_1_MAX){
        insert_seg_free_list_helper(bp, 0);
    }
    else if (LIST_2_MIN <= asize && asize <= LIST_2_MAX){
        insert_seg_free_list_helper(bp, 1);
    }
    else if (LIST_3_MIN <= asize && asize <= LIST_3_MAX){
        insert_seg_free_list_helper(bp, 2);
    }
    else //(LIST_4_MIN <= asize){
        insert_seg_free_list_helper(bp, 3);
    //}
    return NULL;
}

void *remove_seg_free_list_helper(void *bp, int n){
    sf_free_header *ptr = bp;
     if (ptr -> prev != NULL && ptr -> next != NULL){    //not head or tail
        ptr -> prev ->next = ptr -> next;
        ptr -> next -> prev = ptr ->prev;
     }

     else if (ptr -> prev == NULL && ptr -> next != NULL){
        seg_free_list[n].head = ptr -> next;
        seg_free_list[n].head -> prev = NULL;
        seg_free_list[n].head -> next = ptr -> next -> next;

     }

     else if (ptr -> prev != NULL && ptr -> next == NULL){
        ptr -> prev ->next = NULL;
        ptr ->prev = NULL;
     }

     else //(ptr -> prev == NULL && ptr -> next == NULL){
        seg_free_list[n].head = NULL;

     //}

     return NULL;

}


void *remove_seg_free_list(void *bp){
    sf_free_header *ptr = bp;
    uint64_t asize = ptr -> header.block_size << 4;
    if (LIST_1_MIN <= asize && asize <= LIST_1_MAX){
        remove_seg_free_list_helper(bp, 0);
    }
    else if (LIST_2_MIN <= asize && asize <= LIST_2_MAX){
        remove_seg_free_list_helper(bp, 1);
    }
    else if (LIST_3_MIN <= asize && asize <= LIST_3_MAX){
        remove_seg_free_list_helper(bp, 2);
    }
    else // (LIST_4_MIN <= asize){
        remove_seg_free_list_helper(bp, 3);

    //}
    return NULL;
}

void* find_fit(size_t asize){
    asize = asize + 16;      //add footer and header
    sf_free_header *ptr = NULL;
    int n = -1;
    if (LIST_1_MIN <= asize && asize <= LIST_1_MAX){
        n = 0;
    }
    if (LIST_2_MIN <= asize && asize <= LIST_2_MAX){
        n = 1;
    }
    if (LIST_3_MIN <= asize && asize <= LIST_3_MAX){
        n = 2;
    }
    if (LIST_4_MIN <= asize){
        n = 3;
    }
    while (n <4){
        ptr = seg_free_list[n].head;
        while (ptr != NULL){
            if (ptr->header.allocated == 0 && (ptr->header.block_size<<4 >= asize)){
                return ptr;
            }
            ptr = ptr -> next;
        }
        n++;
    }

    return NULL;
}

void *place(void *p, uint64_t size){
    sf_free_header *ptr = p;
    uint64_t requested_size = size;
    uint64_t block_size, padded, allocated;
    allocated = 1;

    uint64_t old_block_size = ptr -> header.block_size << 4;
    //printf("old_block_size %lu\n", old_block_size);
    //printf("requested_size %lu\n", requested_size);
    if (old_block_size < requested_size + 32){    //splinter or just fit the size
        padded = 1;
        block_size = old_block_size;
        write_block(ptr, block_size, requested_size, allocated, padded);
        remove_seg_free_list(ptr);
    }
    else{                                           //split old block: alloctated + free
        if (requested_size % 16 == 0){
            padded = 0;
            block_size = requested_size + 16;   //16 byte for footer and header
        }
        else{
            padded = 1;
            block_size = (requested_size / 16) *16 + 16 +16;
        }
        //printf("actually block_size %lu\n", block_size);
        remove_seg_free_list(ptr);
        write_block(ptr, block_size, requested_size, allocated, padded);   //allocated

        write_block(p + block_size, old_block_size - block_size, 0, 0, 0);  //free, attention: p not ptr
        insert_seg_free_list(p + block_size);


    }

    return NULL;

}

void *coalescing_with_higher_address(void *ptr){
    sf_free_header *p = ptr;
    uint64_t current_size = p ->header.block_size << 4;
    sf_header *next_header = ptr + (p -> header.block_size << 4);
    if (next_header -> allocated == 1 || ptr + (p -> header.block_size << 4) == end_of_heap){
        return NULL;
    }
    else{
        remove_seg_free_list(next_header);

        uint64_t new_block_size = current_size + (next_header->block_size << 4);
        write_block(p, new_block_size, 0, 0, 0);
        return p;
    }
    return NULL;
}

void *coalescing_with_lower_address(void *ptr){
    sf_footer *prev_footer = ptr - SF_FOOTER_SIZE / 8;
    sf_header *prev_header = ptr - (prev_footer -> block_size << 4);

    if (prev_header -> allocated != 0){
        return NULL;
    }
    else{
        remove_seg_free_list(prev_header);
        coalescing_with_higher_address(prev_header);
        insert_seg_free_list(prev_header);
        return prev_header;
    }
}

void *extend_heap(){
    void *old_end_of_heap = get_heap_end();
    if (start_of_heap == 0){
        sf_sbrk();
        start_of_heap = get_heap_start();
        end_of_heap = get_heap_end();
        write_block(start_of_heap, 4096, 0, 0, 0);
        insert_seg_free_list(start_of_heap);
        return start_of_heap;
    }

    else{
        if(sf_sbrk() != (void *) - 1){
            start_of_heap = get_heap_start();
            end_of_heap = get_heap_end();
            write_block(old_end_of_heap, 4096, 0, 0, 0);
            insert_seg_free_list(old_end_of_heap);
            coalescing_with_lower_address(old_end_of_heap);
            /*if (prev_header != NULL){
                sf_snapshot();
                //insert_seg_free_list(prev_header);  //don't need to insert, prev free block already in sf_free_list
            }*/
            return old_end_of_heap;
        }
        else{
            return NULL;
        }
    }

}

void *sf_malloc(size_t size) {
    if (size > PAGE * 4 || size <= 0){
        sf_errno = EINVAL;
        return NULL;
    }
    sf_free_header *p;
    while((p = find_fit(size)) == NULL){
        if(extend_heap() == NULL){
            //printf("don't have enought space\n");
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    place(p, size);
    void *rtn = p;
    rtn += 8;
	return rtn;
}

void *sf_realloc(void *ptr, size_t size) {
    ptr = ptr - 8;   //go to header

    sf_free_header *p = ptr;     //check ptr valid
    sf_footer *footer;
    if (ptr == NULL || ptr < start_of_heap || ptr > end_of_heap || p -> header.allocated == 0){
        sf_mem_fini();
        //printf("realloc check 1 failed.");
        abort();
    }
    footer = ptr + (p -> header.block_size << 4) - SF_FOOTER_SIZE/8;
    if (p -> header.padded != footer -> padded || p -> header.allocated != footer -> allocated){
        sf_mem_fini();
        //printf("realloc check 2 failed.");
        abort();
    }

    if (footer -> requested_size + 16 != footer -> block_size << 4){
        if (footer -> padded != 1){
            sf_mem_fini();
            //printf("realloc check 3 failed.");
            abort();
        }
    }

    if(size == 0){
        sf_free(ptr + 8);
        return NULL;
    }

    sf_free_header *old_block = ptr;
    uint64_t old_block_size = old_block -> header.block_size << 4;
    //sf_blockprint(old_block);
    void *new_block;
    uint64_t current_size, padded;

    if (old_block_size - 16 < size){
        if ((new_block = sf_malloc(size)) == NULL)
            return NULL;
        memcpy(new_block, ptr + 8, old_block_size - 16);
        sf_free(ptr + 8);
        return new_block;
    }
    else if (old_block_size - 16 > size){
        if(old_block_size - 16 - size < 32){   //splinter
            write_block(ptr, old_block_size, size, 1, 1);
        }
        else{
            if (size % 16 == 0){
                padded = 0;
                current_size = size + 16;   //16 byte for footer and header
            }
            else{
                padded = 1;
                current_size = (size / 16) *16 + 16 +16;
            }

            write_block(ptr, current_size, size, 1, padded);
            write_block(ptr + current_size, old_block_size - current_size, 1, 1, 1);
            sf_free(ptr + current_size + 8);
        }
        return ptr + 8;
    }
    else{
        if (size % 16 == 0){
                padded = 0;
            }
            else{
                padded = 1;
            }
        write_block(ptr, old_block_size, size, 1, padded);
        return ptr + 8;
    }

}

void sf_free(void *ptr) {
    ptr = ptr - 8;   //to header
    sf_free_header *p = ptr;
    sf_footer *footer;
    if (ptr == NULL || ptr < start_of_heap || ptr > end_of_heap || p -> header.allocated == 0){
        sf_mem_fini();
        //printf("free check 1 failed.");
        abort();
    }
    footer = ptr + (p -> header.block_size << 4) - SF_FOOTER_SIZE/8;
    if (p -> header.padded != footer -> padded || p -> header.allocated != footer -> allocated){
        sf_mem_fini();
        //printf("free check 2 failed.");
        abort();
    }

    if (footer -> requested_size + 16 != footer -> block_size << 4){
        if (footer -> padded != 1){
            sf_mem_fini();
            //printf("free check 3 failed.");
            abort();
        }
    }

    write_block(p, p -> header.block_size << 4, 0, 0, 0);

    coalescing_with_higher_address(p);

    insert_seg_free_list(p);

}

/*
void test_sf_free_list(){
    extend_heap();
    sf_free_header *p1 = find_fit(300);
    place(p1, 300);
    sf_snapshot();

    sf_free_header *p2 = find_fit(13);
    place(p2, 13);
    //sf_snapshot();

    sf_free_header *p3 = find_fit(3500);
    place(p3, 3500);
    //sf_snapshot();

    sf_free(p3);
    sf_free(p2);
    sf_free(p1);

    sf_snapshot();

}

void test_extand_heap(){
    extend_heap();
    sf_free_header *p1 = find_fit(2048 - 16);
    place(p1, 2048 - 16);
    sf_snapshot();

    sf_free_header *p2 = find_fit(2048 - 16);
    place(p2, 2048 - 16);

    sf_blockprint(p1);
    sf_free(p1);

    extend_heap();

    sf_snapshot();
    return;
}

void test_sf_malloc(){
    void *p1 = sf_malloc(100);
    void *p2 = sf_malloc(5333);
    sf_blockprint(p1 - 8);
    sf_blockprint(p2 - 8);
    sf_free(p1);
    //sf_free(p1);

    sf_snapshot();
}

void test_sf_realloc(){
    void *p1 = sf_malloc(100);
    //void *p3 = sf_malloc(300);

    void *p2 = sf_realloc(p1, 300);
    sf_blockprint(p1 - 8);
    sf_blockprint(p2 - 8);

    sf_snapshot();
}
*/