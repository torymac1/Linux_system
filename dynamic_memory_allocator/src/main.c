#include <stdio.h>
#include "sf_help.h"

int main(int argc, char const *argv[]) {
/*
    sf_mem_init();

    double* ptr = sf_malloc(sizeof(double));

    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);

    sf_mem_fini(); //123123
*/
    sf_mem_init();

    //sf_malloc(100);

    //test_sf_free_list();

    //test_extand_heap();

    //test_sf_malloc();

    //test_sf_realloc();
    //printf("%p    %p\n", start_of_heap, end_of_heap);

    sf_mem_fini();

    //char *p = "12";
    //uint64_t a = 320;
    //printf("p = %p\n", p);
    //printf("p + a = %p\n", p + a);

    return EXIT_SUCCESS;
}
