#include "terminals.h"
#include "sys_calls.h"
#include "scheduler.h"
#include "lib.h"

/* Global Terminal array */
// term_t* terminals[NUM_OF_TERMINALS] = {0x0, 0x0, 0x0};
int curr_total_pcbs = 0;

/*
* create_terminals()
* Input: NONE
* Output: NONE
* This function initializes the 3 terminals by calling create_new_term, and setting the first
* terminal to active
*/
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
    terminals[0]->in_use = 1;
    processes[0]->active = 1;
    /* Execute a new shell for the terminal 1 shell */
    // clear();
    // update_cursor(0,0);
    // execute((uint8_t*)"shell");
}

/*
* create_new_term()
*Input: term_index->which terminal to make (0,1,2)
*Output: None
* This function creates a term_t struct and initializes all of the elements in the struct.
* This is called by create_terminals() to make all the terminals
*/
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

/*
*get_term_by_index()
*Input: index->which terminal to get (0,1,2)
*Output: term_t * corresponding to which terminal we asked for
* This simple function gets the terminal based on its index
*/
term_t* get_term_by_index(int32_t index){
    if (index < 0 || index >= NUM_OF_TERMINALS) return NULL;
    return terminals[index];
}

/*
* get_curr_terminal()
*Input: NONE
*Output: term_t *  corresponding to the active (displayed) terminal
*This function gets the active terminal to tell other functions which one is in use
*/
term_t* get_curr_terminal() {
    int i;
    for (i = 0; i < NUM_OF_TERMINALS; i++) {
        if (terminals[i] != 0x0){
          if (terminals[i]->in_use == 1) return terminals[i];
        }
    }

    return NULL;
}

/*
*copy_screen_text()
*Input: t -> the terminal that we want to copy the screen into
*Output: NONE
*This function copies whatever is in video memory into the fake video corresponding to the
*terminal in the parameter. This is used by scheduling so that a process can be correctly
*displayed when switching between terminals
*/
void copy_screen_text(term_t* terminal) {
    if (terminal == 0x0)
        return;

    char* video_mem = (char *)VID_MEM_START;
    // int32_t i;
    // for (i = 0; i < VID_ROWS * VID_COLS; i++) {
    //     terminal->screen_text[i] = *(uint8_t *)(video_mem + (i << 1));
    // }
    terminal->cursor_x = get_x_cursor();
    terminal->cursor_y = get_y_cursor();
    memcpy(terminal->vid_mem, (uint8_t*)video_mem, 2*VID_MEM_SIZE);
}

/*
*print_screen_text()
*Input: t -> the terminal that we want to write to the screen
*Output: NONE
*This function copies what is saved in the fake video memory corresponding to the terminal
*to video memory so that it can be displayed. This is used by scheduling so that a process can
*be displayed when switching between terminals
*/
void print_screen_text(term_t* terminal) {
    cli();
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
    /* Update the cursor */
    update_cursor(terminal->cursor_x, terminal->cursor_y);
    memcpy((uint8_t*)video_mem, terminal->vid_mem, 2*VID_MEM_SIZE);
    sti();
}

/*
*total_pcbs_created()
*Input: num->what to add to the global num of pcbs variable
*Output: changed num_of_pcbs based on  input
*This function modifies the global num_of_pcbs variable so that other functions can use it. It
*returns the updated num_of_pcbs
*/
int total_pcbs_created(int num) {
    return curr_total_pcbs += num;
}

/*
*set_terminal_pcb()
*Input: pcb->pcb to set to the terminal
*Output: NONE
*This function assigns the given pcb to the active (displayed) terminal and updates the total
* num_of_pcbs
*/
void set_terminal_pcb(pcb_t* pcb) {
    if (pcb == NULL)
        return;

    term_t* curr_terminal = get_curr_terminal();

    process_t* curr_process = get_process_by_index(curr_terminal->term_index);
    int i;
    for (i = 0; i < PROCESSES_PER_TERM; i++) {
        if (curr_terminal->pcb_processes[i] == 0x0) {
            curr_terminal->pcb_processes[i] = pcb;
            /* update the currrent process */
            // processes[process_idx]->curr_pcb = pcb;
            curr_process->curr_pcb = pcb;
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

/*
*switch_terminal()
*Input: old_term ->current_term, new_term->term to switch to
*Output: NONE
*This function is called when ALT+(1/2/3) is called to switch between terminals. It copies the
*screen from the old term, sets the old term to inactive and the new term to active, and then
*prints the saved screen from the new term
*/
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
    // uint8_t* screen_start;
    // vidmap(&screen_start);
    // page_dir_init_fourkb((uint32_t)screen_start, (uint32_t)terminals[new_term]->vid_mem);
    sti();
}
