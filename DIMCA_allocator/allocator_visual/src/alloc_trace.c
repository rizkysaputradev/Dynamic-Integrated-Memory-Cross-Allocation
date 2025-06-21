#include <stdio.h>
#include "alloc_trace.h"
#include "dimca_visual_allocator.h"  // assuming this contains DataCapsule and constants

extern AdvancedMemoryLevel mem_levels[MAX_VISUAL_LEVELS];

void write_allocation_trace(DataCapsule* capsules, int count, const char* filename, AdvancedMemoryLevel* mem_levels) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open file");
        return;
    }

    fprintf(f, "{\n  \"levels\": [\n");
    for (int lvl = 0; lvl < MAX_VISUAL_LEVELS; lvl++) {
        fprintf(f, "    { \"total_size\": %zu, \"blocks\": [\n", mem_levels[lvl].total_size);

        int first = 1;
        for (int i = 0; i < count; i++) {
            if (capsules[i].preferred_level - 1 == lvl && capsules[i].address) {
                if (!first) fprintf(f, ",\n");
                first = 0;

                long offset = (char*)capsules[i].address - (char*)mem_levels[lvl].base;

                fprintf(f,
                    "      { \"offset\": %ld, \"size\": %d, \"free\": false }",
                    offset, capsules[i].size
                );
            }
        }

        fprintf(f, "\n    ] }%s\n", lvl < MAX_VISUAL_LEVELS - 1 ? "," : "");
    }
    fprintf(f, "  ]\n}\n");

    fclose(f);
}


