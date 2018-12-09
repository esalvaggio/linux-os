#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include "types.h"
#include "sys_calls.h"

#define MAX_PROCESSES     3

typedef struct process_struct {
    struct process_struct *next;
    int32_t index;
    pcb_t* curr_pcb;
    int32_t rtc_frequency;
    int8_t in_use;
    int8_t active;
} process_t;

process_t* processes[MAX_PROCESSES];

void switch_processes();
void set_up_processes();
process_t* get_curr_process();
#endif
