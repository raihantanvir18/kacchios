/* memory.h - Simple heap and stack manager */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kalloc_stack(size_t size);
void kfree_stack(void* stack_top, size_t size);

#endif
