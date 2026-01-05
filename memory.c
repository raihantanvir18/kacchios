/* memory.c - Simple heap and stack manager */
#include "memory.h"

#define HEAP_SIZE (512 * 1024) /* 512KB heap for dynamic allocations */

typedef struct mem_block {
    size_t size;
    struct mem_block* next;
    int free;
} mem_block_t;

static uint8_t* heap_start = 0;
static uint8_t* heap_end = 0;
static mem_block_t* free_list = 0;

static size_t align16(size_t value) {
    return (value + 15) & ~((size_t)15);
}

void memory_init(void) {
    extern uint8_t __kernel_end;
    uintptr_t start = (uintptr_t)&__kernel_end;

    /* Align heap start to 4KB boundary to keep things simple */
    start = (start + 0xFFF) & ~((uintptr_t)0xFFF);

    heap_start = (uint8_t*)start;
    heap_end = heap_start + HEAP_SIZE;

    free_list = (mem_block_t*)heap_start;
    free_list->size = HEAP_SIZE - sizeof(mem_block_t);
    free_list->next = 0;
    free_list->free = 1;
}

static void split_block(mem_block_t* block, size_t size) {
    /* Only split if the remaining space can hold another block and some payload */
    if (block->size >= size + sizeof(mem_block_t) + 16) {
        mem_block_t* new_block = (mem_block_t*)((uint8_t*)block + sizeof(mem_block_t) + size);
        new_block->size = block->size - size - sizeof(mem_block_t);
        new_block->next = block->next;
        new_block->free = 1;

        block->size = size;
        block->next = new_block;
    }
}

void* kmalloc(size_t size) {
    size = align16(size);

    mem_block_t* current = free_list;

    while (current) {
        if (current->free && current->size >= size) {
            split_block(current, size);
            current->free = 0;
            return (uint8_t*)current + sizeof(mem_block_t);
        }
        current = current->next;
    }

    /* Out of memory */
    return 0;
}

static void coalesce(void) {
    mem_block_t* current = free_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
            continue;
        }
        current = current->next;
    }
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    block->free = 1;
    coalesce();
}

void* kalloc_stack(size_t size) {
    uint8_t* base = (uint8_t*)kmalloc(size);
    if (!base) {
        return 0;
    }
    /* Return the logical top-of-stack (stack grows down) */
    return base + size;
}

void kfree_stack(void* stack_top, size_t size) {
    if (!stack_top || size == 0) {
        return;
    }
    uint8_t* base = ((uint8_t*)stack_top) - size;
    kfree(base);
}
