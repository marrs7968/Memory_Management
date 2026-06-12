#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "oslabs.h"

#define NULLBLOCK (struct MEMORY_BLOCK){0, 0, 0, 0}

// Insert helper function adapted from Google AI
void insert_at(struct MEMORY_BLOCK *arr, int *size, int target_index, struct MEMORY_BLOCK block) {
    // Shift elements using memmove
    // Destination: target + 1 index, Source: target index
    // Size to move: number of elements from target_index to the current end
    memmove(&arr[target_index + 1], &arr[target_index], (*size - target_index) * sizeof(struct MEMORY_BLOCK));
    
    arr[target_index] = block;
    (*size)++;
}

// ============= BEST FIT =============
struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    if (request_size <= 0) {
        struct MEMORY_BLOCK invalid_block = {-1, -1, -1, -1};
        return invalid_block;
    }
    int closest_size_diff = INT_MAX;
    int closest_size_index = 0;
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else if (memory_map[i].segment_size > request_size) {
                if ((memory_map[i].segment_size - request_size) < closest_size_diff) {
                    closest_size_diff = memory_map[i].segment_size - request_size;
                    closest_size_index = i;
                    continue;
                }
            }
        }
    }
    if (closest_size_diff < INT_MAX) {
        int alloc_start = memory_map[closest_size_index].start_address;
        int alloc_end = alloc_start + request_size - 1;

        int free_size = memory_map[closest_size_index].segment_size - request_size;
        int free_start = alloc_end + 1;
        int free_end = free_start + free_size - 1;

        struct MEMORY_BLOCK allocated_block = {alloc_start, alloc_end, request_size, process_id};
        struct MEMORY_BLOCK free_block = {free_start, free_end, free_size, 0};
        
        insert_at(memory_map, map_cnt, closest_size_index, allocated_block);
        insert_at(memory_map, map_cnt, closest_size_index + 1, free_block);
        // accounting for the original free block, shifting elements left and decrementing
        memmove(&memory_map[closest_size_index + 2], &memory_map[closest_size_index + 3], (*map_cnt - (closest_size_index + 3)) * sizeof(struct MEMORY_BLOCK));
        (*map_cnt)--;

        return allocated_block;
    }
    return NULLBLOCK;
}

// ============= FIRST FIT =============
struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) {
    if (request_size <= 0) {
        struct MEMORY_BLOCK invalid_block = {-1, -1, -1, -1};
        return invalid_block;
    }
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else if (memory_map[i].segment_size > request_size) {
                int alloc_start = memory_map[i].start_address;
                int alloc_end = alloc_start + request_size - 1;

                int free_size = memory_map[i].segment_size - request_size;
                int free_start = alloc_end + 1;
                int free_end = free_start + free_size - 1;

                struct MEMORY_BLOCK allocated_block = {alloc_start, alloc_end, request_size, process_id};
                struct MEMORY_BLOCK free_block = {free_start, free_end, free_size, 0};
                
                insert_at(memory_map, map_cnt, i, allocated_block);
                insert_at(memory_map, map_cnt, i + 1, free_block);
                // accounting for the original free block, shifting elements left and decrementing
                memmove(&memory_map[i + 2], &memory_map[i + 3], (*map_cnt - (i + 3)) * sizeof(struct MEMORY_BLOCK));
                (*map_cnt)--;

                return allocated_block;
            }
        }
    }
    return NULLBLOCK;
}

// ============= WORST FIT =============
struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    if (request_size <= 0) {
        struct MEMORY_BLOCK invalid_block = {-1, -1, -1, -1};
        return invalid_block;
    }
    int largest_size = INT_MIN;
    int largest_size_index = 0;
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else if (memory_map[i].segment_size > request_size) {
                if (memory_map[i].segment_size > largest_size) {
                    largest_size = memory_map[i].segment_size;
                    largest_size_index = i;
                    continue;
                }
            }
        }
    }
    if (largest_size > INT_MIN) {
        int alloc_start = memory_map[largest_size_index].start_address;
        int alloc_end = alloc_start + request_size - 1;

        int free_size = memory_map[largest_size_index].segment_size - request_size;
        int free_start = alloc_end + 1;
        int free_end = free_start + free_size - 1;

        struct MEMORY_BLOCK allocated_block = {alloc_start, alloc_end, request_size, process_id};
        struct MEMORY_BLOCK free_block = {free_start, free_end, free_size, 0};
        
        insert_at(memory_map, map_cnt, largest_size_index, allocated_block);
        insert_at(memory_map, map_cnt, largest_size_index + 1, free_block);
        // accounting for the original free block, shifting elements left and decrementing
        memmove(&memory_map[largest_size_index + 2], &memory_map[largest_size_index + 3], (*map_cnt - (largest_size_index + 3)) * sizeof(struct MEMORY_BLOCK));
        (*map_cnt)--;

        return allocated_block;
    }
    return NULLBLOCK;
}

// ============= NEXT FIT =============
struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    if (request_size <= 0) {
        struct MEMORY_BLOCK invalid_block = {-1, -1, -1, -1};
        return invalid_block;
    }
    for (int i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 
            && memory_map[i].segment_size >= request_size
            && memory_map[i].start_address >= last_address) {
            if (memory_map[i].segment_size == request_size) {
                memory_map[i].process_id = process_id;
                return memory_map[i];
            } else if (memory_map[i].segment_size > request_size) {
                int alloc_start = memory_map[i].start_address;
                int alloc_end = alloc_start + request_size - 1;

                int free_size = memory_map[i].segment_size - request_size;
                int free_start = alloc_end + 1;
                int free_end = free_start + free_size - 1;

                struct MEMORY_BLOCK allocated_block = {alloc_start, alloc_end, request_size, process_id};
                struct MEMORY_BLOCK free_block = {free_start, free_end, free_size, 0};
                
                insert_at(memory_map, map_cnt, i, allocated_block);
                insert_at(memory_map, map_cnt, i + 1, free_block);
                // accounting for the original free block, shifting elements left and decrementing
                memmove(&memory_map[i + 2], &memory_map[i + 3], (*map_cnt - (i + 3)) * sizeof(struct MEMORY_BLOCK));
                (*map_cnt)--;

                return allocated_block;
            }
        }
    }
    return NULLBLOCK;
}

void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt) {
}