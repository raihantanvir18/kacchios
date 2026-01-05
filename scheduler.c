/* scheduler.c - Round-robin scheduler with simple aging */
#include "scheduler.h"
#include "serial.h"

#define DEFAULT_QUANTUM 5

static process_t* ready_queue[MAX_PROCESSES];
static int rq_count = 0;
static uint32_t quantum_ticks = DEFAULT_QUANTUM;
static context_t bootstrap_ctx;

/* Assembly context switch */
extern void ctx_switch(context_t* old_ctx, context_t* new_ctx);

static void enqueue_ready(process_t* p) {
    if (rq_count >= MAX_PROCESSES) {
        return;
    }
    ready_queue[rq_count++] = p;
}

static void age_ready_queue(void) {
    for (int i = 0; i < rq_count; i++) {
        if (ready_queue[i]->age < 250) {
            ready_queue[i]->age++;
        }
    }
}

static process_t* pick_next(void) {
    if (rq_count == 0) {
        return 0;
    }

    int best_index = 0;
    int best_score = ready_queue[0]->priority + ready_queue[0]->age;

    for (int i = 1; i < rq_count; i++) {
        int score = ready_queue[i]->priority + ready_queue[i]->age;
        if (score > best_score) {
            best_score = score;
            best_index = i;
        }
    }

    process_t* chosen = ready_queue[best_index];

    /* Remove chosen from queue */
    for (int i = best_index; i < rq_count - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    rq_count--;

    /* Age remaining processes */
    age_ready_queue();

    chosen->age = 0;
    return chosen;
}

void scheduler_init(uint32_t time_quantum) {
    rq_count = 0;
    quantum_ticks = (time_quantum == 0) ? DEFAULT_QUANTUM : time_quantum;
}

void scheduler_add_ready(process_t* p) {
    if (!p) {
        return;
    }
    p->state = PROC_READY;
    p->time_slice = quantum_ticks;
    enqueue_ready(p);
}

void scheduler_start(void) {
    process_reap_terminated();
    process_t* next = pick_next();
    if (!next) {
        serial_puts("No runnable process. Halting.\n");
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    process_set_current(next);
    next->state = PROC_CURRENT;
    next->time_slice = quantum_ticks;

    ctx_switch(&bootstrap_ctx, &next->ctx);
}

void scheduler_yield(void) {
    process_t* current = process_current();
    process_reap_terminated();

    process_t* next = pick_next();
    if (!next) {
        /* Nothing else to run */
        if (current && current->state == PROC_CURRENT) {
            current->time_slice = quantum_ticks;
        }
        return;
    }

    if (current && current->state == PROC_CURRENT) {
        current->state = PROC_READY;
        current->time_slice = quantum_ticks;
        enqueue_ready(current);
    }

    process_set_current(next);
    next->state = PROC_CURRENT;
    next->time_slice = quantum_ticks;

    if (current) {
        ctx_switch(&current->ctx, &next->ctx);
    } else {
        ctx_switch(&bootstrap_ctx, &next->ctx);
    }
}

void scheduler_tick(void) {
    process_t* current = process_current();
    if (!current) {
        scheduler_yield();
        return;
    }

    if (current->time_slice > 0) {
        current->time_slice--;
    }

    if (current->time_slice == 0) {
        scheduler_yield();
    }
}

void scheduler_terminate_current(void) {
    process_t* current = process_current();
    if (!current) {
        return;
    }

    process_reap_terminated();
    process_t* next = pick_next();

    if (!next) {
        serial_puts("All processes terminated. Halting.\n");
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    current->state = PROC_TERMINATED;
    current->needs_reap = 1;

    process_set_current(next);
    next->state = PROC_CURRENT;
    next->time_slice = quantum_ticks;

    ctx_switch(&current->ctx, &next->ctx);
}

__asm__ (
".globl ctx_switch\n"
"ctx_switch:\n"
"    mov 4(%esp), %eax\n"     /* old_ctx */
"    mov 8(%esp), %edx\n"     /* new_ctx */
"    mov %ebx, 0(%eax)\n"     /* save callee-saved regs */
"    mov %ebp, 4(%eax)\n"
"    mov %esi, 8(%eax)\n"
"    mov %edi, 12(%eax)\n"
"    mov %esp, 16(%eax)\n"    /* save stack pointer */
"    mov 16(%edx), %esp\n"    /* restore stack pointer */
"    mov 0(%edx), %ebx\n"     /* restore callee-saved regs */
"    mov 4(%edx), %ebp\n"
"    mov 8(%edx), %esi\n"
"    mov 12(%edx), %edi\n"
"    ret\n"                    /* return into resumed process */
);
