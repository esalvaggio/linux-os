#ifndef _TERMINALS_H
#define _TERMINALS_H

#include "types.h"
#include "sys_calls.h"

#define VID_ROWS            80
#define VID_COLS            25
#define VID_MEM_SIZE        2000
#define VID_MEM_START       0xB8000
#define TEXT_COLOR1         0x4E
#define TEXT_COLOR2         0x1C
#define TEXT_COLOR3         0x24
#define NUM_OF_TERMINALS    3
/* Max processes per terminal, including initial shell */
#define PROCESSES_PER_TERM  4

typedef struct terminal {
    int term_index;
    int num_of_pcbs;
    int visited;
    uint8_t screen_text[VID_MEM_SIZE];
    int32_t cursor_x;
    int32_t cursor_y;
    pcb_t* pcb_processes[PROCESSES_PER_TERM];
    int8_t in_use;
} term_t;

void create_terminals();
void create_new_term(int term_index);
term_t* get_curr_terminal();
void copy_screen_text(term_t* terminal);
void print_screen_text(term_t* terminal);
int total_pcbs_created();
void set_terminal_pcb(pcb_t* pcb);
void switch_terminal(int old_term, int new_term);

#endif
