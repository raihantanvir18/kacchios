/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128

static void null_process(void* arg) {
    char input[MAX_INPUT];
    int pos = 0;

    (void)arg;

    serial_puts("Running null process...\n\n");

    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;

        /* Read input line */
        while (1) {
            char c = serial_getc();

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
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
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