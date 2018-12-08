#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include "types.h"

void switch_processes();

typedef struct process_struct {
    struct process_struct *next;
} process_t;
#endif
