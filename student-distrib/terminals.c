#include "terminals.h"
#include "sys_calls.h"
#include "lib.h"

/* Global Terminal array */
term_t* terminals[NUM_OF_TERMINALS] = {0x0, 0x0, 0x0};
int curr_total_pcbs = 0;

void create_terminals() {

    int i;
    for (i = 0; i < NUM_OF_TERMINALS; i++)
    {
        create_new_term(i);
    }
    /* Set the F1 terminal to be in use on start-up */
    terminals[0]->in_use = 1;
    /* Execute a new shell the terminal 1 shell */
    clear();
    update_cursor(0,0);
    execute((uint8_t*)"shell");
}

void create_new_term(int term_index) {

    if (term_index < 0 || term_index >= NUM_OF_TERMINALS) //invalid terminal number
        return;

    if (terminals[term_index] != 0x0) //checks if terminal already exists
        return;

    term_t* new_terminal = (term_t*)(ADDR_8MB - (term_index+7)*ADDR_8KB);
    new_terminal->in_use = 0;
    new_terminal->term_index = term_index;
    new_terminal->visited = 0;
    new_terminal->num_of_pcbs = 0;
    new_terminal->cursor_x = 0;
    new_terminal->cursor_y = 0;
    new_terminal->screen_text[0] = '\0';
    terminals[term_index] = new_terminal;

    int i;
    for(i = 0; i < PROCESSES_PER_TERM; i++)
        new_terminal->pcb_processes[i] = 0x0;

    /* Update our total number of pcb's used */
    curr_total_pcbs++;
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
    /* Update the video memory with the terminal's buffer */
    for (i = 0; i < VID_ROWS * VID_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = terminal->screen_text[i];
        if(terminal->term_index == 0){
          *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR1;
        }else if (terminal->term_index == 1){
          *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR2;
        }else if(terminal->term_index == 2){
          *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR3;
        }
    }
    /* Update the cursor */
    update_cursor(terminal->cursor_x, terminal->cursor_y);
}

int total_pcbs_created(int num) {
    return curr_total_pcbs += num;
}

void set_terminal_pcb(pcb_t* pcb) {
    if (pcb == NULL)
        return;

    term_t* curr_terminal = get_curr_terminal();

    int i;
    for (i = 0; i < PROCESSES_PER_TERM; i++) {
        if (curr_terminal->pcb_processes[i] == 0x0) {
            curr_terminal->pcb_processes[i] = pcb;
            break;
        }
    }

    curr_terminal->num_of_pcbs++;
    /* Update whether the terminal has been visited */
    if (curr_terminal->visited != 1) {
        curr_terminal->visited = 1;
        return;
    }
    /* Only if the terminal has been visited, then we update this */
    curr_total_pcbs++;
}

void switch_terminal(int old_term, int new_term) {
    if (old_term == new_term)
        return;
    else if (new_term < 0 || new_term >= NUM_OF_TERMINALS)
        return;
    else if (terminals[new_term] == 0x0)
        return;
    /* Save the currently used terminal's text screen */
    copy_screen_text(terminals[old_term]);
    /* Set the new terminal to be in use */
    terminals[old_term]->in_use = 0;
    terminals[new_term]->in_use = 1;
    /* Execute a new shell once we go to a new terminal for the first time */
    if (terminals[new_term]->visited != 1) {
        sti();
        clear();
        update_cursor(0,0);
        execute((uint8_t*)"shell");
    }
    /* Print the data of the terminal we are switching to */
    print_screen_text(terminals[new_term]);
    sti();
}
