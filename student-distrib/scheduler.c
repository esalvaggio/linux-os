#include "scheduler.h"
#include "terminals.h"
#include "devices/keyboard.h"
#include "paging.h"

#define PROCESS_OFFSET          10
/*
  This might all be wrong.
*/

/*
*switch_processes()
*Input:NONE
*Output: NONE
*This function is the main scheduling function: it changes the current process, modifies the TSS, restores paging to the new process, and then context switches to the next process by modifying the ebp/esp
*/
void switch_processes(){
    /* Reset the video memory */
    uint8_t* screen_start;
    vidmap(&screen_start);

    process_t* curr_process = get_curr_process();
    if (curr_process == 0x0)
        return;
    process_t* next_process = curr_process->next;
    /* Null check */
    if (next_process == 0x0)
        return;

    curr_process->in_use = 0;
    next_process->in_use = 1;

    pcb_t* curr_pcb = curr_process->curr_pcb;
    if (curr_pcb == 0x0)
        return;
    pcb_t* next_pcb = next_process->curr_pcb;
    if (next_pcb == 0x0)
        return;
    /* Update the tss to the process we are switching to */

    tss.esp0 = ADDR_8MB - ADDR_8KB*(next_pcb->process_num) - FOUR_BYTE_ADDR;   /* MAY NEED TO CHANGE THIS */
    tss.ss0 = KERNEL_DS;


    uint32_t phys_addr = ADDR_8MB + (next_pcb->process_num * ADDR_4MB);
    page_dir_init(VIRTUAL_ADDRESS, phys_addr);

    // Save EBP and ESP
    asm volatile ("                         \n\
                    movl %%ebp, %0          \n\
                    movl %%esp, %1          \n\
                    "
                    :"=r"(curr_pcb->ebp), "=r"(curr_pcb->esp)
                    :
                  );
  // Update the EBP and ESP
    asm volatile ("                         \n\
                    movl %0, %%ebp          \n\
                    movl %1, %%esp          \n\
                    "
                    :
                    :"r"(next_pcb->ebp), "r"(next_pcb->esp)
                  );
    flushTLB();
}

/*
*set_up_processes()
*Input:NONE
*Output:NONE
*This function initializes all of the elements of the process struct and links them together as a
*linked list
*/
void set_up_processes() {
    /* Initialize process pointer array */
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        processes[i] = 0x0;
    }

    for (i = 0; i < MAX_PROCESSES; i++) {
        process_t* p = (process_t*)(ADDR_8MB - (i+PROCESS_OFFSET)*ADDR_8KB);
        p->index = i;
        p->rtc_frequency = 2;
        p->in_use = 0;
        p->curr_pcb = 0x0;
        p->active = 0;
        processes[i] = p;
    }
    //need to do another loop for next, cause next will always be null in
    //the previous loop
    for (i = 0; i < MAX_PROCESSES; i++){
        process_t* p = processes[i];
        int next_idx = (i+1)%MAX_PROCESSES;
        p->next = processes[next_idx];
    }
    /* Set the first process to be in use to begin */
    processes[0]->in_use = 1;
    curr_process = 0;
    next_process = 0;
}

/*
*get_process_by_index()
*Input: index -> the index desired
*Output: process_t * that has the index in the parameter
*This function returns the process that has the index requested. This is called when a pcb wants
* get a process by its terminal index
*/
process_t* get_process_by_index(int32_t index){
    if (index <0 || index >= MAX_PROCESSES) return NULL;
    return processes[index];
}

/*
*get_curr_process()
*Input: NONE
*Output: process_t * that is currently active
*This function finds the process that is currently being handled and returns it so that other
*functions can use its contents
*/
process_t* get_curr_process() {
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] != 0x0) {
            if (processes[i]->in_use == 1)
                return processes[i];
        }
    }

    return 0x0;
}
