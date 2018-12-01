#ifndef _TERMINALS_H
#define _TERMINALS_H

#include "types.h"

#define VID_ROWS            80
#define VID_COLS            25
#define VID_MEM_SIZE        2000
#define VID_MEM_START       0xB8000
#define NUM_OF_TERMINALS    3
/* Max processes per terminal, excluding initial shell */
#define MAX_PROCESSES       3

typedef struct terminal {
    int term_index;
    uint8_t screen_text[VID_MEM_SIZE];
    int32_t cursor_x;
    int32_t cursor_y;
    int8_t pcb_indices[MAX_PROCESSES];
    int8_t in_use;
} term_t;

void create_terminals();
void create_new_term(int term_index);
term_t* get_curr_terminal();
void copy_screen_text();
void switch_terminal(int old_term, int new_term);

#endif
