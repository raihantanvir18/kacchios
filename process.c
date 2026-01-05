/* process.c - Process table and creation */
#include "process.h"
#include "scheduler.h"
#include "string.h"
#include "serial.h"

static process_t proc_table[MAX_PROCESSES];
static int next_pid = 1;
static process_t* current = 0;

static void process_trampoline(void);

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        proc_table[i].pid = -1;
        proc_table[i].state = PROC_TERMINATED;
        proc_table[i].in_use = 0;
        proc_table[i].needs_reap = 0;
        proc_table[i].age = 0;
        proc_table[i].time_slice = 0;
    }
    next_pid = 1;
    current = 0;
}

process_t* process_current(void) {
    return current;
}

void process_set_current(process_t* p) {
    current = p;
    if (p) {
        p->state = PROC_CURRENT;
    }
}

process_t* process_get(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].in_use && proc_table[i].pid == pid) {
            return &proc_table[i];
        }
    }
    return 0;
}

static process_t* allocate_slot(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!proc_table[i].in_use) {
            proc_table[i].in_use = 1;
            return &proc_table[i];
        }
    }
    return 0;
}

int process_create(const char* name, process_entry_t entry, void* arg, size_t stack_size, uint8_t priority) {
    process_t* p = allocate_slot();
    if (!p) {
        return -1;
    }

    if (!stack_size) {
        stack_size = 4096; /* default 4KB stack */
    }

    uint8_t* stack_top = (uint8_t*)kalloc_stack(stack_size);
    if (!stack_top) {
        p->in_use = 0;
        return -1;
    }

    /* Initialize PCB */
    p->pid = next_pid++;
    p->state = PROC_READY;
    p->priority = priority;
    p->age = 0;
    p->time_slice = 0;
    p->entry = entry;
    p->arg = arg;
    p->stack_size = stack_size;
    p->stack_base = stack_top - stack_size;
    p->needs_reap = 0;

    /* Build initial context */
    p->ctx.ebx = 0;
    p->ctx.ebp = 0;
    p->ctx.esi = 0;
    p->ctx.edi = 0;
    p->ctx.eflags = 0x2; /* Keep interrupts disabled; bit 1 must stay set */
    p->ctx.eip = (uint32_t)process_trampoline; /* informational; ctx_switch returns via stack */

    /*
     * ctx_switch() resumes a process via 'ret', so the initial stack must contain
     * a return address pointing at process_trampoline.
     */
    uint32_t* sp = (uint32_t*)stack_top;
    *(--sp) = (uint32_t)process_trampoline;
    p->ctx.esp = (uint32_t)sp;

    /* Copy name */
    int i = 0;
    while (name && name[i] && i < PROCESS_NAME_LEN - 1) {
        p->name[i] = name[i];
        i++;
    }
    p->name[i] = '\0';

    scheduler_add_ready(p);
    return p->pid;
}

void process_reap_terminated(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t* p = &proc_table[i];
        if (p->in_use && p->state == PROC_TERMINATED && p->needs_reap) {
            serial_puts("Reaping PID ");
            serial_put_uint((uint32_t)p->pid);
            serial_puts(" (stack freed)\n");
            if (p->stack_base) {
                kfree(p->stack_base);
            }
            p->in_use = 0;
            p->needs_reap = 0;
        }
    }
}

static const char* state_str(process_state_t state) {
    switch (state) {
        case PROC_CURRENT: return "CURRENT";
        case PROC_READY: return "READY";
        case PROC_BLOCKED: return "BLOCKED";
        case PROC_TERMINATED: return "TERMINATED";
        default: return "?";
    }
}

void process_dump(void) {
    serial_puts("PID  STATE        NAME\n");
    serial_puts("------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t* p = &proc_table[i];
        if (!p->in_use) {
            continue;
        }

        serial_put_uint((uint32_t)p->pid);
        serial_puts("  ");
        serial_puts(state_str(p->state));

        /* crude spacing */
        if (p->state == PROC_READY) serial_puts("       ");
        else if (p->state == PROC_CURRENT) serial_puts("     ");
        else if (p->state == PROC_BLOCKED) serial_puts("     ");
        else serial_puts("  ");

        serial_puts("  ");
        serial_puts(p->name);
        serial_puts("\n");
    }
}

void process_exit(void) {
    process_t* p = process_current();
    if (!p) {
        return;
    }

    p->state = PROC_TERMINATED;
    p->needs_reap = 1;

    serial_puts("Process terminated: ");
    serial_puts(p->name);
    serial_puts("\n");

    scheduler_terminate_current();

    /* Should not reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void process_trampoline(void) {
    process_t* p = process_current();
    if (p && p->entry) {
        p->entry(p->arg);
    }
    process_exit();
}
