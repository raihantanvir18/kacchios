/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128

static void busy_delay(uint32_t loops) {
    for (uint32_t i = 0; i < loops; i++) {
        __asm__ volatile ("nop");
    }
}

static void worker_process(void* arg) {
    const char* tag = (const char*)arg;
    uint32_t count = 0;

    serial_puts("[worker ");
    serial_puts(tag);
    serial_puts("] started\n");

    while (count < 30) {
        serial_puts("[worker ");
        serial_puts(tag);
        serial_puts("] tick ");
        serial_put_uint(count);
        serial_puts("\n");

        count++;
        busy_delay(300000);
        scheduler_yield();
    }

    serial_puts("[worker ");
    serial_puts(tag);
    serial_puts("] exiting\n");
    process_exit();
}

static void short_lived_process(void* arg) {
    (void)arg;

    serial_puts("[short] allocating heap... ");
    void* mem = kmalloc(64);
    serial_puts("ptr=");
    serial_put_hex32((uint32_t)(uintptr_t)mem);
    serial_puts("\n");

    if (mem) {
        kfree(mem);
        serial_puts("[short] freed heap block\n");
    }

    serial_puts("[short] exiting now\n");
    process_exit();
}

static void run_demo(void) {
    serial_puts("\n--- DEMO START ---\n");
    serial_puts("Creating 3 processes: worker A, worker B, short\n");

    int pid_a = process_create("workA", worker_process, (void*)"A", 4096, 1);
    int pid_b = process_create("workB", worker_process, (void*)"B", 4096, 1);
    int pid_s = process_create("short", short_lived_process, 0, 4096, 2);

    serial_puts("Created PIDs: A=");
    serial_put_uint((uint32_t)pid_a);
    serial_puts(" B=");
    serial_put_uint((uint32_t)pid_b);
    serial_puts(" short=");
    serial_put_uint((uint32_t)pid_s);
    serial_puts("\n");

    serial_puts("Type 'ps' to see table; watch interleaving output.\n");
    serial_puts("--- DEMO RUNNING ---\n\n");
}

static void null_process(void* arg) {
    char input[MAX_INPUT];
    int pos = 0;

    (void)arg;

    serial_puts("Running null process...\n\n");

    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;

        /* Read input line (non-blocking so other processes can run) */
        while (1) {
            char c;
            if (!serial_try_getc(&c)) {
                scheduler_yield();
                continue;
            }

            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            } else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");
            } else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);
            }
        }

        if (pos > 0) {
            if (strcmp(input, "help") == 0) {
                serial_puts("Commands: help, ps, demo\n");
            } else if (strcmp(input, "ps") == 0) {
                process_dump();
            } else if (strcmp(input, "demo") == 0) {
                run_demo();
            } else {
                serial_puts("You typed: ");
                serial_puts(input);
                serial_puts("\n");
            }
        }

        /* Cooperative yield to let other processes run */
        scheduler_yield();
    }
}

void kmain(void) {
    /* Initialize hardware */
    serial_init();
    memory_init();
    process_init();
    scheduler_init(5);

    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");

    /* Create the null process and start scheduling */
    process_create("null", null_process, 0, 4096, 1);
    scheduler_start();

    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}