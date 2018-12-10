#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include "types.h"
#include "sys_calls.h"

#define MAX_PROCESSES     3

typedef struct process_struct {
    struct process_struct *next;
    int32_t index;  //in the process array
    pcb_t* curr_pcb;
    int32_t rtc_frequency;
    int8_t in_use;
    int8_t active;
} process_t;

process_t* processes[MAX_PROCESSES];
int curr_process;
int next_process;

void switch_processes();
void set_up_processes();
process_t* get_curr_process();
process_t* get_process_by_index(int32_t index);
#endif
