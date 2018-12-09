#include "terminals.h"
#include "sys_calls.h"
#include "scheduler.h"
#include "lib.h"

/* Global Terminal array */
// term_t* terminals[NUM_OF_TERMINALS] = {0x0, 0x0, 0x0};
int curr_total_pcbs = 0;

void create_terminals() {
    int i;
    for (i = 0; i < NUM_OF_TERMINALS; i++)
    {
        terminals[i] = 0x0;
    }

    for (i = 0; i < NUM_OF_TERMINALS; i++)
    {
        create_new_term(i);
    }
    /* Set the F3 terminal to be in use because we start the shells from
      right to left */
    terminals[2]->in_use = 1;
    processes[0]->active = 1;
    /* Execute a new shell for the terminal 1 shell */
    // clear();
    // update_cursor(0,0);
    // execute((uint8_t*)"shell");
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
    new_terminal->vid_mem = (uint8_t*)(VID_MEM_START + (term_index+1)*(ADDR_4KB));
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
    // int32_t i;
    // for (i = 0; i < VID_ROWS * VID_COLS; i++) {
    //     terminal->screen_text[i] = *(uint8_t *)(video_mem + (i << 1));
    // }

    memcpy(terminal->vid_mem, (uint8_t*)video_mem, 2*VID_MEM_SIZE);

    terminal->cursor_x = get_x_cursor();
    terminal->cursor_y = get_y_cursor();
}

void print_screen_text(term_t* terminal) {
    if (terminal == 0x0)
        return;

    /* Reset our cursors so we print from the beginning */
    // update_cursor(0,0);
    char* video_mem = (char *)VID_MEM_START;
    // int32_t i;
    // /* Update the video memory with the terminal's buffer */
    // for (i = 0; i < VID_ROWS * VID_COLS; i++) {
    //     *(uint8_t *)(video_mem + (i << 1)) = terminal->screen_text[i];
    //     if(terminal->term_index == 0){
    //       *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR1;
    //     }else if (terminal->term_index == 1){
    //       *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR2;
    //     }else if(terminal->term_index == 2){
    //       *(uint8_t *)(video_mem + (i << 1) + 1) = TEXT_COLOR3;
    //     }
    // }
    memcpy((uint8_t*)video_mem, terminal->vid_mem, 2*VID_MEM_SIZE);

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
    int process_idx = curr_terminal->term_index;
    int i;
    for (i = 0; i < PROCESSES_PER_TERM; i++) {
        if (curr_terminal->pcb_processes[i] == 0x0) {
            curr_terminal->pcb_processes[i] = pcb;
            /* update the currrent process */
            processes[process_idx]->curr_pcb = pcb;
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
    cli();
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
    /* Update the active process */
    processes[old_term]->active = 0;
    processes[new_term]->active = 1;
    /* Execute a new shell once we go to a new terminal for the first time */
    if (terminals[new_term]->visited != 1) {
        /* Get the current pcb of the old terminal */
        pcb_t* old_pcb;
        int i;
        for (i = 0; i < PROCESSES_PER_TERM; i++) {
            if (terminals[old_term]->pcb_processes[i] != 0x0) {
                if (terminals[old_term]->pcb_processes[i]->in_use == 1) {
                    old_pcb = terminals[old_term]->pcb_processes[i];
                    break;
                }
            }
        }


        clear();
        update_cursor(0,0);
        /* Save the EBP and ESP of the old pcb before switching away */
        asm volatile ("                         \n\
                        movl %%ebp, %0          \n\
                        movl %%esp, %1          \n\
                        "
                        :"=r"(old_pcb->ebp), "=r"(old_pcb->esp)
                        :
                      );
        sti();
        execute((uint8_t*)"shell");
    }
    /* Print the data of the terminal we are switching to */
    print_screen_text(terminals[new_term]);
    sti();
}
