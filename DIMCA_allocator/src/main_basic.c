
#include "dimca_basic_allocator.h"
#include <stdio.h>

int main() {
    init_memory_levels();

    double cost1, cost2, cost3;

    void* ptr1 = cross_malloc(40, 1, &cost1);
    printf("Allocated 40 bytes at Level 1 -> ptr1 = %p, cost = %.2f\n", ptr1, cost1);

    void* ptr2 = cross_malloc(70, 1, &cost2);
    printf("Allocated 70 bytes at Level 1 -> ptr2 = %p, cost = %.2f\n", ptr2, cost2);

    void* ptr3 = cross_malloc(120, 3, &cost3);
    printf("Allocated 120 bytes at Level 3 -> ptr3 = %p, cost = %.2f\n", ptr3, cost3);

    cross_free(ptr1);
    printf("Freed ptr1\n");

    cross_free(ptr2);
    printf("Freed ptr2\n");

    cross_free(ptr3);
    printf("Freed ptr3\n");

    return 0;
}
