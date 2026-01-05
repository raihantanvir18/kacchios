/* serial.h - Serial port driver interface */
#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* str);
char serial_getc(void);

/* Non-blocking input: returns 1 if a char was read, else 0 */
int serial_try_getc(char* out);

/* Minimal number formatting helpers */
void serial_put_uint(uint32_t value);
void serial_put_hex32(uint32_t value);

#endif