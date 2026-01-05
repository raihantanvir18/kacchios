/* scheduler.h - Simple round-robin scheduler */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void scheduler_init(uint32_t time_quantum);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_tick(void);
void scheduler_add_ready(process_t* p);
void scheduler_terminate_current(void);

#endif
