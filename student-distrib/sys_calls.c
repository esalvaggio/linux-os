#include "sys_calls.h"
#include "terminals.h"
#include "fs_setup.h"
#include "devices/rtc.h"
#include "devices/keyboard.h"
#include "paging.h"
#include "x86_desc.h"

/* Filetypes */
#define RTC_FILETYPE         0
#define DIR_FILETYPE         1
#define REG_FILETYPE         2
/* Characters for executable checks */
#define EXEC_CHECK_CHARS     4
#define DELETE_CHAR       0x7F
#define E_CHAR            0x45
#define L_CHAR            0x4C
#define F_CHAR            0x46
/* Memory sizes */
#define ADDR_8MB      0x800000
#define ADDR_4MB      0x400000
#define ADDR_8KB      0x002000
#define ADDR_4KB      0x001000
/* File system numbers*/
#define FILE_ENTRY  0x08048000
#define DYNAMIC_FILE_START   2
#define ENTRY_POINT         24
#define ENTRY_POINT_END     28
#define MAX_SHELLS           2
/* Return values */
#define SUCCESS              0
#define ERROR               -1

/* Global PCB file array */
pcb_t* pcb_processes[NUM_OF_PROCESSES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

//Arrays for file operations pointer table
fotp_t file_funcs = {file_open, file_close, file_read, file_write};
fotp_t rtc_funcs = {RTC_open, RTC_close, RTC_read, RTC_write};
fotp_t key_funcs = {Terminal_Open, Terminal_Close, Terminal_Read, Terminal_Write};
fotp_t stdin_func = {NULL, NULL, Terminal_Read, NULL};
fotp_t stdout_func = {NULL, NULL, NULL, Terminal_Write};
fotp_t dir_funcs = {dir_open, dir_close, dir_read, dir_write};
fotp_t no_fotp = {NULL, NULL, NULL, NULL};
int8_t executable_check[EXEC_CHECK_CHARS] = {DELETE_CHAR, E_CHAR, L_CHAR, F_CHAR};
// first 4 bytes (0x7f, 0x45, 0x4c, 0x46)
uint32_t user_stack_pointer = VIRTUAL_ADDRESS + STACK_PAGE_SIZE - FOUR_BYTE_ADDR;

/* Keeps track of the current number of shells */
int curr_num_of_shells = 0;

/*
 * find_new_process()
 * INPUTS: None
 * OUTPUTS: Index of first open pcb slot
 * Function iterates through the global pcb_processes Array
 * and finds the first open slot (called in execute when making new pcb)
*/
int32_t find_new_process() {
    int i;
    for (i = 0; i < NUM_OF_PROCESSES; i++) {
        if (pcb_processes[i] == 0x0) {
          /* Found an open pcb */
          return i;
        }
    }
    /* No pcb's open */
    return ERROR;
}

/*
 * create_new_pcb()
 * INPUTS: process_num is the index of the new pcb in the array
 * OUTPUTS: pointer to new pcb
 * This function takes the index of where the new pcb is to be located
 * in the pcb array and creates a new pcb (setting all files to unused)
 * Side effects: new pcb is written into memory at location (8MB-(process*8kb))
*/
pcb_t* create_new_pcb(int32_t process_num) {

    /* Starts at address: 8MB - (process_num * 8KB) */
    pcb_t* new_pcb = (pcb_t*)(ADDR_8MB - (process_num+1)*ADDR_8KB);
    new_pcb->process_num = process_num;
    new_pcb->mem_addr_start = ADDR_8MB - (process_num+1)*ADDR_8KB;

    int fa_index;
    /* Initialize all files to unused */
    for (fa_index = DYNAMIC_FILE_START; fa_index < FILE_ARRAY_SIZE; fa_index++){
      /* Initialize each location in file array to unused */
      new_pcb->file_array[fa_index].flags = 0;
      new_pcb->file_array[fa_index].inode = -1;
      new_pcb->file_array[fa_index].file_pos = 0;
      new_pcb->file_array[fa_index].file_ops_table_ptr = no_fotp;
    }
    /* Initialize stdin and stdout to in use and bound to terminal functions*/
    for (fa_index = 0; fa_index < DYNAMIC_FILE_START; fa_index++){
      new_pcb->file_array[fa_index].flags = 1;
      new_pcb->file_array[fa_index].inode = 0;
      new_pcb->file_array[fa_index].file_pos = 0;
      //new_pcb->file_array[fa_index].file_ops_table_ptr = key_funcs;
      if(fa_index == 0)
      {
        new_pcb->file_array[fa_index].file_ops_table_ptr = stdin_func;
      }
      else if(fa_index == 1)
      {
        new_pcb->file_array[fa_index].file_ops_table_ptr = stdout_func;
      }
    }
    //Default is no parent
    new_pcb->parent_pcb = 0x0;
    new_pcb->args[0] = '\0';

    return new_pcb;
}
/*
 * halt()
 * INPUTS: NONE
 * OUTPUTS: NONE (jumps to execute)
 * halt essentially tears down the current pcb and hands off control to the
 * parent pcb. It also has to re enable paging for the previous pcb and re set
 * ebp and the tss values.
 * SIDE EFFECTS: changes ebp to what its value was at beginning of execute.
 *               changes tss values to what they were in execute
 *               removes current pcb
*/
int32_t halt(uint8_t status) {
    /* Check if we are halting from the base shell. If we are,
     * then we want to restart the shell but not leave from it.
     */
    cli();
    process_t* curr_process = get_curr_process();
    pcb_t* curr_pcb = curr_process->curr_pcb;
    if (curr_pcb == 0x0) return ERROR;
    term_t* curr_terminal = get_term_by_index(curr_process->index);
    int i;

    /* Check if we are trying to halt from our base shell in terminal */


    if (curr_terminal->num_of_pcbs == 1) {
          int pcb_index = curr_terminal->pcb_processes[0]->process_num;
          pcb_processes[pcb_index] = NULL;
          curr_terminal->pcb_processes[0] = NULL;
          curr_process->curr_pcb = NULL;
          curr_terminal->num_of_pcbs--;
          (void)total_pcbs_created(-1);
          execute((uint8_t*)"shell");
    }
    else {


    //printf("Not logical value at line number %d in file %s\n", __LINE__, __FILE__);
    /*
      1. Restore parent data
        - parent process number (most important)
          -> look in PCB for this
    jumps into execute, restores eip
    */
    //int index;
    int current_num = 0;
    int old_num = 0;
    pcb_t * pcb_parent;



    if (curr_pcb->parent_pcb != 0x0) {
        current_num = curr_pcb->process_num;
        pcb_parent = (curr_pcb->parent_pcb);
        old_num = pcb_parent->process_num;

        /* 3. Clear file descriptors */

        for (i = DYNAMIC_FILE_START; i < FILE_ARRAY_SIZE; i++){
            /* close all files in the pcb */
            close(i);
        }

        curr_pcb->in_use = 0;
        pcb_parent->in_use = 1;
    }

    /*
          2. Restore parent paging
            - similar to how we set up paging in execute()
            - flush TLB!
    */
    uint32_t old_addr = ADDR_8MB + (old_num * ADDR_4MB);
    page_dir_init(VIRTUAL_ADDRESS, old_addr); //set the paging back to 8MB

      /*
      4. Jump back to parent process
    */

    // step 4
    // We need to return to the parent esp/ebp that should be saved in the pcb structure
    tss.esp0 = pcb_processes[old_num]->esp0;
    tss.ss0 = pcb_processes[old_num]->ss0; //kernel stack segment = kernel_DS


    for(i = 0; i < 4; i++)
    {
        if(curr_terminal->pcb_processes[i] == curr_pcb)
        {
            curr_terminal->pcb_processes[i] = NULL;
            curr_terminal->pcb_processes[i-1]->in_use = 1;
            break;
        }
    }

    pcb_processes[current_num] = 0x0;
    curr_process->curr_pcb = pcb_parent;
    curr_pcb = 0x0;
    curr_terminal->num_of_pcbs--;
    (void)total_pcbs_created(-1);

    /* Obtain the old_ebp. The value stored at this ebp is the one we want to use
      to return back. */
    uint32_t* old_ebp;
    asm volatile ("                         \n\
                    movl %%ebp, %0          \n\
                    "
                    :"=r"(old_ebp)
                    :
                  );

    /* Dereference the pointer and update the current ebp. */
    asm volatile ("                         \n\
                    movl %0, %%ebp          \n\
                    movl %1, %%eax          \n\
                    jmp END_OF_EXECUTE      \n\
                    "
                    :
                    : "r"(*old_ebp), "r"((uint32_t)status)
                );

    //Will never reach, need to include regardless
}
    return ERROR;
}
/*
 * execute()
 * INPUTS: command is a const pointer to an array of characters to be executed
 * OUTPUTS: EAX will store the result of the execute. (Will most likely be 0)
 *
 * Execute is a monster of a function. First, it parses the input command and
 * stores the command and its args in two new character buffers.
 * Then, it takes the first 4 characters and compares them to the valid command
 * list. It then enables paging and creates a new pcb to run the command.
*/
int32_t execute(const uint8_t* command) {
    if(command == NULL) return ERROR;
    cli();

    //Find new process index
    int32_t process_num = find_new_process();
    term_t* curr_terminal = get_curr_terminal();
    /* Instructions
      1. Parse  ---
          - command: ["filename" + " " + "string of args"]
    */
    int32_t command_idx;
    int32_t arg_buf_len = 0;
    int32_t command_length = strlen((int8_t*)command);
    uint8_t* fname;
    uint8_t * args = NULL;
    uint8_t arg_buf[ARGS_LEN];
    uint8_t buf[FILENAME_SIZE];

    uint8_t space_flag = 0;

    for (command_idx = 0; command_idx < command_length; command_idx++) {
        if (command[command_idx] == ' ') {
            space_flag = 1;
            /* Get filename */
            if (command_length >= ARGS_LEN){
                sti();
                return ERROR;
            }
            copy_string(buf, command, command_idx);
            buf[command_idx] = '\0';
            fname = buf;
            /* Get arguments */
            arg_buf_len = command_length - (command_idx + 1);
            copy_string(arg_buf, &command[command_idx + 1], arg_buf_len);
            arg_buf[arg_buf_len] = '\0';
            args = &arg_buf[0]; //get address of first char in args
            break;
        }
    }
    //special case if there is no arguments
    if (space_flag == 0){
        // We don't want to overflow the buffer and cause a pagefault
        if (command_length > FILENAME_SIZE){
            sti();
            return ERROR;
        }
        copy_string(buf, command, command_length);
        buf[command_length] = '\0';
        fname = buf;
    }

    /*  2. Executable Check   ---
          - check if file is an executable
            -> read dentry
            -> get data of file, check for "\dELF"
            -> 40 bytes of file is a header
              -> first 4 bytes (0x7f, 0x45, 0x4c, 0x46)
              -> entry point bytes 24-27
    */
    dentry_t dentry;
    if (read_dentry_by_name(fname, &dentry) < 0) {
          sti();
          return ERROR;
      }

    /* Check if we have reached the max processes for one terminal */
    if (curr_terminal->visited == 1) {
      if (total_pcbs_created(0) == NUM_OF_PROCESSES) {
          printf("Maximum processes running... Cannot execute.\n");
          sti();
          return 0;
      }
    }

    //ex_buf stores first 4 bytes to check it is a valid command
    int8_t ex_buf[EXEC_CHECK_CHARS];
    if (read_data(dentry.inode_num, 0, ex_buf, EXEC_CHECK_CHARS) < EXEC_CHECK_CHARS) {
        sti();
        return ERROR;
    }
    //Make sure entered buffer is a valid command - first 4 bytes certain chars
    for (command_idx = 0; command_idx < EXEC_CHECK_CHARS; command_idx++) {
        if (ex_buf[command_idx] != executable_check[command_idx]) {
            sti();
            return ERROR;
        }
    }
    /* get entry point from bytes 24-27 */
    if (read_data(dentry.inode_num, ENTRY_POINT, ex_buf, ENTRY_POINT_END) < 0) {
        sti();
        return ERROR;
    }
    /* Address of first instruction */
    uint32_t entry_point = *((uint32_t*)ex_buf);

    /*
      3. Paging
          - each process gets its own 4 MB page */
    uint32_t phys_addr = ADDR_8MB + (process_num * ADDR_4MB);
    page_dir_init(VIRTUAL_ADDRESS, phys_addr);
    //printf("EXECUTE line number %d in file %s\n", __LINE__, __FILE__);


    /*
      4. User-level program loader
          - call to read_data
          - comes after paging!
          - all user level programs will be loaded in the page starting at 128MB
          - virtual memory => 128 MB - 132 MB
              -> gets mapped to physical memory
              -> physical memory starts at 8 MB + (process number)*4 MB
          - flush TLB
              -> reload a control register (CR3?)
              -> after initializing a new page
    */
    inode_t* inode = (inode_t*)(boot_block + dentry.inode_num + 1);

    if (read_data(dentry.inode_num, 0, (int8_t*)FILE_ENTRY, inode->length) < 0){
        sti();
        return ERROR;
    }

    /* Flush TLB by writing to CR3 */
    flushTLB();

    /*
      5. create PCB
          - args should be put into PCB
          - allocate space for PCB in memory
          - total kernel stack goes from 4MB - 8MB
            -> kernel stack for process 1 => 8MB -> (8MB - 8KB)
                -> very bottom of kernel stack
            -> kernel stack for process 2 => (8MB - 8KB) -> (8MB - 8KB) - 8KB
                -> above previous process kernel stack
            -> ... so on
    */
    pcb_t * pcb_new = create_new_pcb(process_num);
    pcb_new->term_index = curr_terminal->term_index;
    pcb_processes[process_num] = pcb_new;
    /* Update the pcb array in our current terminal */

    pcb_t* curr_pcb = get_pcb_ptr();

    /* First, check if there is a process running already. Because we are
    only running one shell, the running process is the parent of the process
    being called right now. If no processes are found, no parent exists
    */

    if (curr_pcb != NULL) {
        pcb_new->parent_pcb = curr_pcb;
        curr_pcb->in_use = 0;
    }


    pcb_new->in_use = 1;
    /* Copy arguments into pcb */
    if(space_flag == 1)
    {
      copy_string(pcb_new->args, arg_buf, ARGS_LEN);
    }

    /*
      6. context switch
          - change priviledge level
          - IRET (read in ISA manual)
            -> user DS
            -> esp
            -> eflag
            -> cs
            -> etp
          - push artificial IRET onto the stack
            -> IRET pops 32B from the stack, so use PUSHL
            -> push the new (user) ESP and segment selector
          - should not be able to leave shell!!
            -> first user-level program called in kernel.c
    */

    tss.esp0 = ADDR_8MB - ADDR_8KB*(process_num) - FOUR_BYTE_ADDR;
    tss.ss0 = KERNEL_DS; //kernel stack segment = kernel_DS

    pcb_new->esp0 = tss.esp0;
    pcb_new->ss0 = tss.ss0;

    //Next few lines save current ebp into the process control block.
    // This is needed when we eventually jump back to this function + return
    uint32_t curr_ebp;
    uint32_t curr_esp;
    asm volatile("                           \n\
                  movl %%ebp, %0             \n\
                  movl %%esp, %1             \n\
                  "
                  : "=r"(curr_ebp), "=r"(curr_esp)
                );

   pcb_new->ebp = curr_ebp;
   pcb_new->esp = curr_esp;
   set_terminal_pcb(pcb_new);

    /* Explanation of following assembly code:
       We need to push 0x2B (which is the USER_DS defined in x86_desc.h:15)
       into ax, ds, es, fs, and gs to properly set up the iret context.

       Then we need to push that value onto the stack, along with:
            user_stack_pointer
            flags
            0x23 (USER_CS, defined in x86_desc.h:14)
            entry_point
       At which case we call iret. Important note about the lines:
                  popl %%eax
                  orl$0x200, %%eax
                  pushl %%eax
          These lines pop the flags back off the stack, and or them with
          the 10th bit (which will call an sti upon the iret, which is
          needed because we have not left the cli defined at the top)
    */


    /*
    HALT will jump to END_OF_EXECUTE tag, and perform
    a leave ret. HALT restores the ebp of execute.
    */
    sti();
    asm volatile ("                         \n\
                    movw $0x2B, %%ax        \n\
                    movw %%ax, %%ds         \n\
                    movw %%ax, %%es         \n\
                    movw %%ax, %%fs         \n\
                    movw %%ax, %%gs         \n\
                    pushl $0x2B             \n\
                    pushl %1                \n\
                    pushfl                  \n\
                    popl %%eax              \n\
                    orl $0x00000200,%%eax   \n\
                    pushl %%eax             \n\
                    pushl $0x23             \n\
                    pushl %0                \n\
                    iret                    \n\
                    END_OF_EXECUTE:         \n\
                    leave                   \n\
                    ret                     \n\
                    "
                    :                 // (no) ouput
                    : "r"(entry_point), "r"(user_stack_pointer)
                    : "eax","edx"    // clobbered register
                  );

    return ERROR; //shouldn't get this far
}
/*
 * read()
 * INPUTS: fd is location of file in pcb's fie array
 *         buf stores the buffer for bytes to be read into
 *         nbytes is the amount of bytes that will be read
 * OUTPUTS: returns the amount of bytes read (success return = nbytes)
 *          or -1 otherwise
 *  Read takes nbytes from the selected file and copies them into the buf
 * SIDE EFFECTS: updates the current file's position
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes) {

    //Check for invalid index
    if (fd < 0 || fd >= FILE_ARRAY_SIZE)
        return ERROR;

    //Get current PCB
    pcb_t * pcb_curr = get_pcb_ptr();
    if (pcb_curr == NULL) return ERROR;

    /* Check file position */
    fd_t file_desc = pcb_curr->file_array[fd];
    //If file descriptor is not open, error
    if (file_desc.flags == 0){
      return ERROR;
    }

    /* Make the correct read call for file type */
    if(file_desc.file_ops_table_ptr.read == NULL)
    {
      return ERROR;
    }

    /* The corresponding read call checks if we have reached the end
        of the file */
    return file_desc.file_ops_table_ptr.read(fd, buf, nbytes);
}
/*
 * write()
 * INPUTS: fd is position of file in file descriptor Array
 *         buf stores the bytes needed to be written to file
 *         nbytes is the number of bytes in the buffer
 * OUTPUTS: Number of bytes written, -1 on error
 * Write a specific amount of bytes to a file. Checks if a valid write, then executes
 * Side effects: pcb is changed to active pcb
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {

    //Check if a valid fd number
    if (fd < 0 || fd >= FILE_ARRAY_SIZE)
        return ERROR;

    pcb_t* pcb_curr = get_pcb_ptr();
    if (pcb_curr == NULL) {
        return ERROR;
    }

    /* Check file position */
    fd_t file_desc = pcb_curr->file_array[fd];
    if (file_desc.flags == 0){
        return ERROR;
    }
    /* Return 0 if we've reached the end of the file */
    /* Make the correct read call for file type */

    if(file_desc.file_ops_table_ptr.write == NULL)
    {
        sti();
        return ERROR;
    }

    return file_desc.file_ops_table_ptr.write(fd, buf, nbytes);
}
/*
 * open()
 * INPUTS: filename is the file we want to open
 * OUTPUTS: 0 on success, -1 on error
 *
 * Open first checks if the file exists in the file system. Then it looks
 * for an open spot in the pcb file array. If so, the file is opened and
 * A link to its inode is placed in the file_desc within the file array
 * and the rest of the file_desc is changed.
 *
 * SIDE EFFECTS:
 *    Upon a successful open, the current pcb's file array is loaded with a
 *    new file descriptor
*/
int32_t open(const uint8_t* filename) {

    dentry_t dentry;
    pcb_t * pcb_curr = get_pcb_ptr();
    if (pcb_curr == NULL) return ERROR;

    //Check if file exists, get it's dentry
    if (read_dentry_by_name(filename, &dentry) < 0)
        return ERROR;

    int fa_index;
    int32_t fd;
    for (fa_index = DYNAMIC_FILE_START; fa_index < FILE_ARRAY_SIZE; fa_index++) {
        /* Find the next open location in file array */
        fd_t file_desc = pcb_curr->file_array[fa_index];
        if (file_desc.flags != 1) {
            /* Inode is 0 for non-data files */
            if (dentry.filetype == REG_FILETYPE) {
                file_desc.inode = dentry.inode_num;
            } else {
                file_desc.inode = 0;
            }
            /* Gets updated every time read is called */
            file_desc.file_pos = 0;
            file_desc.flags = 1;

            switch (dentry.filetype)
            {
                case RTC_FILETYPE:
                    /* set the frequency of the RTC */
                    // call rtc open and set fotp
                    fd = RTC_open(dentry.filename);
                    file_desc.file_ops_table_ptr = rtc_funcs;
                    break;
                case DIR_FILETYPE:
                    // dir_open and set ftop
                    fd = dir_open(dentry.filename);
                    file_desc.file_ops_table_ptr = dir_funcs;
                    break;
                case REG_FILETYPE:
                    // file_open and set fotp
                    fd = file_open(dentry.filename);
                    file_desc.file_ops_table_ptr = file_funcs;
                    break;
            }
            pcb_curr->file_array[fa_index] = file_desc;
            return fa_index;
        }
    }
    /* There are no open locations in the file array */
    return ERROR;
}
/*
 * close()
 * INPUTS: fd is location of file in file array in pcb
 * OUTPUTS: SUCCESS OR ERROR
 * close simply calls a close function on the file called.
 * SIDE EFFECTS: closes the file
*/
int32_t close(int32_t fd) {
    //Check if valid fd number
    if (fd < DYNAMIC_FILE_START || fd >= FILE_ARRAY_SIZE)
        return ERROR;

    pcb_t * pcb_curr = get_pcb_ptr();

    if (pcb_curr->file_array[fd].flags == 1){
      pcb_curr->file_array[fd].flags = 0;

      if(pcb_curr->file_array[fd].file_ops_table_ptr.close == NULL)
      {
        return ERROR;
      }

      return pcb_curr->file_array[fd].file_ops_table_ptr.close(fd);
      // return SUCCESS;
    }
    return ERROR;

}

/*
 * getargs()
 * INPUTS: buf - stores the arguments
 *         nbytes - amount of bytes to read into buf
 * OUTPUTS: -1 if there are no arguments, 0 on success.
 * This functions stores the command line arguments into the buffer provided.
 * We go to the current pcb and retrieve the arguments from there.
*/
int32_t getargs(uint8_t* buf, int32_t nbytes) {

    if(buf == NULL || nbytes < 0) //invalid parameters
        return ERROR;

    pcb_t * curr_pcb = get_pcb_ptr(); //get PCB
    if(curr_pcb->args[0] == '\0') //PCB has no args, failure
        return ERROR;

    int x;
    int args_length = 0;
    while (curr_pcb->args[args_length] != '\0') //calculate length of args
        args_length++;

    if(nbytes < args_length) //if buffer is too small for args, return failure
        return ERROR;

    for(x = 0; x < nbytes; x++)
        buf[x] = curr_pcb->args[x]; //store args into given buffer

    return SUCCESS; //if we made it here it was successful
}

/*
 * vidmap()
 * INPUTS: screen_start - virtual address to where we want to map video memory
 * OUTPUTS: the start of video memory address
 * This function maps the text-mode video memory into user space at a pre-set
 * virtual address. This requires us to create a new 4KB page.
*/
int32_t vidmap(uint8_t** screen_start) {
  if(screen_start == NULL){ //null check
    return ERROR;
  }

  if(screen_start >= (uint8_t **)ADDR_4MB && screen_start <= (uint8_t **)ADDR_8MB) //kernel memory check
  {
    return ERROR;
  }

  page_dir_init_fourkb((uint32_t)VIDMEM_ADDR,(uint32_t)VIDEO);
  *screen_start = (uint8_t*)VIDMEM_ADDR;
  return VIDMEM_ADDR;
}
/*
 * set_handler()
 * -- extra credit --
*/
int32_t set_handler(int32_t signum, void* handler_address) {
    return ERROR;
}

/*
 * sigreturn()
 * -- extra credit --
*/
int32_t sigreturn(void) {
    return ERROR;
}

/*
 * get_pcb_ptr()
 * INPUTS: NONE
 * OUTPUTS: pointer to the current pcb
 * Finds the one active pcb. Nothing special, just used at beginning
 * of every function and figured it would be better to make into a routine.
 * Gets current process's pcb
*/
pcb_t* get_pcb_ptr() {
  process_t* p = get_curr_process();
  return p->curr_pcb;
}
