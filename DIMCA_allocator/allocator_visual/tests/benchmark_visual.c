#include "dimca_visual_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAPSULE_COUNT 1000

void logger_to_file(const char* message) {
    static FILE* file = NULL;
    if (!file) file = fopen("allocation_log.txt", "w");
    fprintf(file, "%s\n", message);
    fflush(file);
}

int random_size() {
    return (rand() % 256) + 16;
}

int random_level(int max_level) {
    return (rand() % max_level) + 1;
}

int main() {
    srand((unsigned int)time(NULL));

    size_t sizes[] = {64 * 1024, 128 * 1024, 256 * 1024};
    int levels = sizeof(sizes) / sizeof(sizes[0]);

    init_visual_memory_levels(sizes, levels, logger_to_file);

    DataCapsule capsules[CAPSULE_COUNT];
    for (int i = 0; i < CAPSULE_COUNT; i++) {
        capsules[i].id = i + 1;
        capsules[i].size = random_size();
        capsules[i].preferred_level = random_level(levels);
    }

    batch_visual_allocate(capsules, CAPSULE_COUNT);

    for (int i = 0; i < CAPSULE_COUNT; i++) {
        if (capsules[i].address)
            visual_free(capsules[i].address);
    }

    shutdown_visual_allocator();
    return 0;
}
