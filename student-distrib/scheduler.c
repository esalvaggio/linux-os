#include "scheduler.h"
#include "terminals.h"
#include "devices/keyboard.h"

/*
  This might all be wrong.
*/

void switch_processes(){
    /* Get the terminal we are switching to */
    term_t* next_terminal = terminals[terminal_fn_key];
    /* Null check and current terminal check */
    if (next_terminal == 0x0)
        return;
    if (next_terminal->in_use == 1)
        return;
    /* Reset the video memory */
    uint8_t* screen_start;
    vidmap(&screen_start);
    /* Get the current process from that terminal */
    pcb_t* next_pcb = 0x0;
    int i;
    for (i = 0; i < PROCESSES_PER_TERM; i++) {
        if (next_terminal->pcb_processes[i] != 0x0) {
            if (next_terminal->pcb_processes[i]->in_use == 1) {
                next_pcb = next_terminal->pcb_processes[i];
                break;
            }
        }
    }
    /* Update the tss to the process we are switching to */
    if (next_pcb != 0x0) {
        tss.esp0 = next_pcb->esp0;   /* MAY NEED TO CHANGE THIS */
        tss.ss0 = KERNEL_DS;
    }
    /* Save EBP and ESP */
    pcb_t* curr_pcb = get_curr_pcb();
    asm volatile ("                         \n\
                    movl %%ebp, %0          \n\
                    movl %%esp, %0          \n\
                    "
                    :"=r"(curr_pcb->ebp), "=r"(curr_pcb->esp)
                    : /* no input */
                  );
    /* Update the EBP and ESP */
    asm volatile ("                         \n\
                    movl %0, %%ebp          \n\
                    movl %0, %%esp          \n\
                    "
                    : /* no outputs */
                    :"r"(next_pcb->ebp), "r"(next_pcb->esp)
                  );
}
