#include "dimca_visual_allocator.h"
#include "allocator_opt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct BlockHeader {
    size_t size;
    int free;
    int level;
    struct BlockHeader* next;
    struct BlockHeader* prev;
} BlockHeader;

typedef struct {
    pthread_mutex_t lock;
    BlockHeader* bins[BIN_COUNT];
    size_t total_size;
    void* base;
} VisualMemoryLevel;

static VisualMemoryLevel levels[MAX_VISUAL_LEVELS];
static int level_count = 0;
static double alpha = 0.5;
static VisualizerLogger logger = NULL;

static int get_bin_index(size_t size) {
    if (size <= 16) return 0;
    if (size <= 32) return 1;
    if (size <= 64) return 2;
    if (size <= 128) return 3;
    return 4;
}

static double calc_cost(int preferred, int actual) {
    return 1.0 + alpha * abs(preferred - actual);
}

void init_visual_memory_levels(size_t* sizes, int count, VisualizerLogger log_fn) {
    logger = log_fn;
    level_count = count > MAX_VISUAL_LEVELS ? MAX_VISUAL_LEVELS : count;
    for (int i = 0; i < level_count; i++) {
        levels[i].base = mmap(NULL, sizes[i], PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (levels[i].base == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }
        levels[i].total_size = sizes[i];
        pthread_mutex_init(&levels[i].lock, NULL);
        for (int j = 0; j < BIN_COUNT; j++) levels[i].bins[j] = NULL;

        BlockHeader* block = (BlockHeader*)levels[i].base;
        block->size = sizes[i] - sizeof(BlockHeader);
        block->free = 1;
        block->level = i + 1;
        block->next = block->prev = NULL;

        levels[i].bins[get_bin_index(block->size)] = block;

        if (logger) {
            char msg[256];
            snprintf(msg, sizeof(msg), "[INIT] Level %d base=%p size=%zu", i + 1, block, block->size);
            logger(msg);
        }
    }
}

void batch_visual_allocate(DataCapsule* capsules, size_t count) {
    for (size_t c = 0; c < count; c++) {
        size_t aligned = fast_align_up(capsules[c].size);
        int bin = get_bin_index(aligned);
        int order[MAX_VISUAL_LEVELS];
        order[0] = capsules[c].preferred_level - 1;
        int idx = 1;
        for (int i = 0; i < level_count; i++)
            if (i != order[0]) order[idx++] = i;

        for (int i = 0; i < level_count; i++) {
            int lvl = order[i];
            VisualMemoryLevel* level = &levels[lvl];
            pthread_mutex_lock(&level->lock);
            for (int b = bin; b < BIN_COUNT; b++) {
                BlockHeader* curr = level->bins[b];
                while (curr) {
                    if (curr->free && curr->size >= aligned) {
                        curr->free = 0;
                        if (curr->size >= aligned + sizeof(BlockHeader) + 16) {
                            BlockHeader* new_block = (BlockHeader*)((char*)curr + sizeof(BlockHeader) + aligned);
                            new_block->size = curr->size - aligned - sizeof(BlockHeader);
                            new_block->free = 1;
                            new_block->level = curr->level;
                            new_block->next = curr->next;
                            new_block->prev = curr;
                            curr->next = new_block;
                            int new_bin = get_bin_index(new_block->size);
                            new_block->next = level->bins[new_bin];
                            level->bins[new_bin] = new_block;
                        }
                        pthread_mutex_unlock(&level->lock);

                        capsules[c].address = (void*)(curr + 1);
                        capsules[c].time_cost = calc_cost(capsules[c].preferred_level, lvl + 1);

                        // Print allocation info here
                        if (logger) {
                            char msg[256];
                            snprintf(msg, sizeof(msg), "[ALLOC] id=%d size=%d preferred=%d actual=%d addr=%p cost=%.2f",
                                     capsules[c].id, capsules[c].size, capsules[c].preferred_level,
                                     lvl + 1, capsules[c].address, capsules[c].time_cost);
                            logger(msg);
                        }

                        goto next_capsule;
                    }
                    curr = curr->next;
                }
            }
            pthread_mutex_unlock(&level->lock);
        }
        capsules[c].address = NULL;
        capsules[c].time_cost = -1.0;
        if (logger) {
            char msg[128];
            snprintf(msg, sizeof(msg), "[FAIL] id=%d size=%d", capsules[c].id, capsules[c].size);
            logger(msg);
        }
    next_capsule:;
    }
}

void visual_free(void* ptr) {
    if (!ptr) return;
    BlockHeader* block = (BlockHeader*)ptr - 1;
    VisualMemoryLevel* level = &levels[block->level - 1];
    pthread_mutex_lock(&level->lock);
    block->free = 1;
    if (block->next && block->next->free) {
        block->size += sizeof(BlockHeader) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(BlockHeader) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
    int bin = get_bin_index(block->size);
    block->next = level->bins[bin];
    level->bins[bin] = block;
    pthread_mutex_unlock(&level->lock);

    if (logger) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[FREE] addr=%p level=%d", ptr, block->level);
        logger(msg);
    }
}

void shutdown_visual_allocator() {
    for (int i = 0; i < level_count; i++) {
        munmap(levels[i].base, levels[i].total_size);
        pthread_mutex_destroy(&levels[i].lock);
    }
    if (logger) logger("[SHUTDOWN]");
}
