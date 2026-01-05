/* process.h - Process management */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "memory.h"

#define MAX_PROCESSES 16
#define PROCESS_NAME_LEN 16

typedef enum {
    PROC_TERMINATED = 0,
    PROC_READY,
    PROC_CURRENT,
    PROC_BLOCKED
} process_state_t;

typedef void (*process_entry_t)(void* arg);

typedef struct context {
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
} context_t;

typedef struct process {
    int pid;
    char name[PROCESS_NAME_LEN];
    process_state_t state;
    uint8_t priority;
    uint8_t age;
    uint32_t time_slice;
    process_entry_t entry;
    void* arg;
    context_t ctx;
    uint8_t* stack_base;   /* start of stack allocation */
    size_t stack_size;
    int in_use;
    int needs_reap;
} process_t;

void process_init(void);
int process_create(const char* name, process_entry_t entry, void* arg, size_t stack_size, uint8_t priority);
void process_exit(void);
process_t* process_current(void);
process_t* process_get(int pid);
void process_set_current(process_t* p);
void process_reap_terminated(void);
void process_dump(void);

#endif
