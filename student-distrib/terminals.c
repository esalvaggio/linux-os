#include "terminals.h"
#include "sys_calls.h"
#include "lib.h"

/* Global Terminal array */
term_t* terminals[NUM_OF_TERMINALS] = {0x0, 0x0, 0x0};

void create_terminals() {
    int i;
    for (i = 0; i < NUM_OF_TERMINALS; i++)
    {
        create_new_term(i);
        //execute((uint8_t *)"shell");
    }

    terminals[0]->in_use = 1;
}

void create_new_term(int term_index) {

    if (term_index < 0 || term_index >= NUM_OF_TERMINALS) //invalid terminal number
        return;

    if (terminals[term_index] != 0x0) //checks if terminal already exists
        return;

    term_t* new_terminal = (term_t*)(ADDR_8MB - (term_index+7)*ADDR_8KB);
    new_terminal->in_use = 1;
    new_terminal->term_index = term_index;
    new_terminal->cursor_x = 0;
    new_terminal->cursor_y = 0;
    new_terminal->pcbs_full = 0;
    new_terminal->screen_text[0] = '\0';
    terminals[term_index] = new_terminal;

    int i;
    for(i = 0; i < PROCESSES_PER_TERM; i++)
        new_terminal->pcb_processes[i] = 0x0;

    /* Execute a new shell once we go to a new terminal for the first time */
    clear();
    update_cursor(0,0);
    execute((uint8_t*)"shell");
}

term_t* get_curr_terminal() {
    int i;
    for (i = 0; i < NUM_OF_TERMINALS; i++) {
        if (terminals[i] != 0x0){
          if (terminals[i]->in_use == 1) return terminals[i];
        }
    }

    return NULL;
}

void copy_screen_text(term_t* terminal) {
    if (terminal == 0x0)
        return;

    char* video_mem = (char *)VID_MEM_START;
    int32_t i;
    for (i = 0; i < VID_ROWS * VID_COLS; i++) {
        terminal->screen_text[i] = *(uint8_t *)(video_mem + (i << 1));
        // *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }

    terminal->cursor_x = get_x_cursor();
    terminal->cursor_y = get_y_cursor();
}

void print_screen_text(term_t* terminal) {
    if (terminal == 0x0)
        return;

    /* Reset our cursors so we print from the beginning */
    update_cursor(0,0);
    char* video_mem = (char *)VID_MEM_START;
    int32_t i;
    for (i = 0; i < VID_ROWS * VID_COLS; i++) {
        // putc(terminal->screen_text[i]);
        *(uint8_t *)(video_mem + (i << 1)) = terminal->screen_text[i];
        *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR;
    }

    update_cursor(terminal->cursor_x, terminal->cursor_y);
}

void set_terminal_pcb(pcb_t* pcb) {
    if (pcb == NULL)
        return;

    term_t* curr_terminal = get_curr_terminal();

    int i;
    for (i = 0; i < PROCESSES_PER_TERM; i++) {
        if (curr_terminal->pcb_processes[i] == 0x0)
            curr_terminal->pcb_processes[i] = pcb;
    }
}

void switch_terminal(int old_term, int new_term) {
    if (old_term == new_term)
        return;
    else if (new_term < 0 || new_term >= NUM_OF_TERMINALS)
        return;

    /* Save the currently used terminal's text screen */
    copy_screen_text(terminals[old_term]);
    terminals[old_term]->in_use = 0;

    if (terminals[new_term] != 0x0) {
        terminals[new_term]->in_use = 1;
    } else {
        sti();
        create_new_term(new_term);
    }

    print_screen_text(terminals[new_term]);
    sti();
}