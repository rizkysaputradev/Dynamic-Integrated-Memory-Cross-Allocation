#include "dimca_visual_allocator.h"
#include <stdio.h>

void console_logger(const char* message) {
    printf("%s\n", message);
}

int main() {
    size_t sizes[] = {64 * 1024, 128 * 1024, 256 * 1024};
    int levels = sizeof(sizes) / sizeof(sizes[0]);

    init_visual_memory_levels(sizes, levels, console_logger);

    DataCapsule capsules[] = {
        {1, 40, 1}, {2, 70, 1}, {3, 120, 3}, {4, 200, 2}, {5, 300, 1}
    };
    size_t count = sizeof(capsules) / sizeof(capsules[0]);

    batch_visual_allocate(capsules, count);

    for (size_t i = 0; i < count; i++) {
        if (capsules[i].address) visual_free(capsules[i].address);
    }

    shutdown_visual_allocator();
    return 0;
}
