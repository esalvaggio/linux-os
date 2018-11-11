#include "sys_calls.h"
#include "fs_setup.h"
#include "devices/rtc.h"
#include "devices/keyboard.h"
#include "paging.h"
#include "x86_desc.h"


#define RTC_FILETYPE         0
#define DIR_FILETYPE         1
#define REG_FILETYPE         2
#define EXEC_CHECK_CHARS     4
#define DELETE_CHAR       0x7F
#define E_CHAR            0x45
#define L_CHAR            0x4C
#define F_CHAR            0x46
#define ADDR_8MB      0x800000
#define ADDR_4MB      0x400000
#define ADDR_8KB      0x002000
#define ADDR_4KB      0x001000
#define FILE_ENTRY  0x08048000
#define DYNAMIC_FILE_START   2
#define ENTRY_POINT         24
#define ENTRY_POINT_END     28
#define SUCCESS              0
#define ERROR               -1
pcb_t* pcb;
pcb_t* pcb_processes[NUM_OF_PROCESSES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

//Arrays for file operations pointer table
fotp_t file_funcs = {file_open, file_close, file_read, file_write};
fotp_t rtc_funcs = {RTC_open, RTC_close, RTC_read, RTC_write};
fotp_t key_funcs = {Terminal_Open, Terminal_Close, Terminal_Read, Terminal_Write};
fotp_t dir_funcs = {dir_open, dir_close, dir_read, dir_write};
int8_t executable_check[EXEC_CHECK_CHARS] = {DELETE_CHAR, E_CHAR, L_CHAR, F_CHAR};
// first 4 bytes (0x7f, 0x45, 0x4c, 0x46)
uint32_t user_stack_pointer = VIRTUAL_ADDRESS + STACK_PAGE_SIZE - FOUR_BYTE_ADDR;

int32_t find_new_process() {
    int i;
    for (i = 0; i < NUM_OF_PROCESSES; i++) {
        if (pcb_processes[i] == NULL) {
          /* Found an open pcb */
          return i;
        }
    }

    /* No pcb's open */
    return ERROR;
}

pcb_t* create_new_pcb(int32_t process_num) {

    /* Starts at address: 8MB - (process_num * 8KB) */
    pcb_t* new_pcb = (pcb_t*)(ADDR_8MB - (process_num+1)*ADDR_8KB);
    new_pcb->process_num = process_num;
    new_pcb->mem_addr_start = ADDR_8MB - (process_num+1)*ADDR_8KB;

    int fa_index;
    for (fa_index = DYNAMIC_FILE_START; fa_index < FILE_ARRAY_SIZE; fa_index++){
      /* Initialize each location in file array to unused */
      new_pcb->file_array[fa_index].flags = 0;
      new_pcb->file_array[fa_index].inode = -1;
      new_pcb->file_array[fa_index].file_pos = 0;
    }
    for (fa_index = 0; fa_index < DYNAMIC_FILE_START; fa_index++){
      new_pcb->file_array[fa_index].flags = 1;
      new_pcb->file_array[fa_index].inode = 0;
      new_pcb->file_array[fa_index].file_pos = 0;
      new_pcb->file_array[fa_index].file_ops_table_ptr = key_funcs;

    }

    new_pcb->parent_pcb = 0x0;

    // set terminal read/write to indexes 0 and 1 here!

    return new_pcb;
}

// void set

int32_t halt(uint8_t status) {
    /*
      1. Restore parent data
        - parent process number (most important)
          -> look in PCB for this*/

          /*
          NONE OF THIS WORKS!!!!!!!
          DONT TAKE ANY OF IT SERIOUSLY
          THE PCB STUFF MIGHT WORK
          WHO KNOWS

          */
    /*
    jumps into execute, restores eip
    */
    int index;
    int current_num = 0;
    int old_num = 0;
    pcb_t * pcb_rent;
    for (index = 0; index < NUM_OF_PROCESSES; index++){
        if (pcb_processes[index] != 0x0){
          if (pcb_processes[index]->in_use == 1){

              if(pcb_processes[index]->parent_pcb == 0x0) //first process does not have a parent so skip parent stuff
              {
                break;
              }
              current_num = pcb_processes[index]->process_num;
              pcb_rent = (pcb_t *)pcb_processes[index]->parent_pcb;
              old_num = pcb_rent->process_num;
              pcb_processes[index]->in_use = 0;
                pcb_rent->in_use = 1;
              break;
          }
        }
    }

    /*
          2. Restore parent paging
            - similar to how we set up paging in execute()
            - flush TLB!
    */
    //uint32_t phys_addr = ADDR_8MB + (process_num * ADDR_4MB);
    page_dir_init(VIRTUAL_ADDRESS, ADDR_8MB); //set the paging back to 8MB
      /*
      3. Clear all file descriptors
        - calling close()
      4. Jump back to parent process

    */

    //3
    int i;
    for (i = DYNAMIC_FILE_START; i < FILE_ARRAY_SIZE; i++){
        (void)pcb_processes[current_num]->file_array[i].file_ops_table_ptr.close;
    }
    //pcb_processes[current_num] = 0x0;
    //4

    //We need to somehow return to the parent esp/ebp that should be saved in the pcb structure
    //Not really sure what to do with them
    tss.esp0 = pcb_processes[old_num]->esp0;
    tss.ss0 = pcb_processes[old_num]->ss0; //kernel stack segment = kernel_DS

    int32_t old_ebp = pcb_processes[current_num]->ebp;
    pcb_processes[current_num] = 0x0;

    asm volatile ("                         \n\
                    movl %0, %%EBP          \n\
                    movl $0, %%eax          \n\
                    jmp END_OF_EXECUTE      \n\
                    "
                    :
                    : "r"(old_ebp)               // (no) ouput
                  );

    return SUCCESS;
}

int32_t execute(const uint8_t* command) {
    cli();

    if(command == NULL)
    {
      return ERROR;
    }
        cli();
    /* Instructions
      1. Parse  --- WORKS
          - command: ["filename" + " " + "string of args"]
    */
    int32_t command_idx, arg_buf_len;
    int32_t command_length = strlen((int8_t*)command);
    uint8_t* fname;
    uint8_t* args;
    uint8_t space_flag = 0;

    for (command_idx = 0; command_idx < command_length; command_idx++) {
        if (command[command_idx] == ' ') {
            space_flag = 1;
            /* Get filename */
            uint8_t buf[FILENAME_SIZE];
            copy_string(buf, command, command_idx);
            buf[command_idx] = '\0';
            fname = buf;

            /* Get arguments */
            arg_buf_len = command_length - (command_idx + 1);
            uint8_t arg_buf[arg_buf_len];
            copy_string(arg_buf, &command[command_idx + 1], arg_buf_len);
            arg_buf[arg_buf_len] = '\0';
            args = arg_buf;
            break;
        }

    }
    if (space_flag == 0){
        uint8_t buf[FILENAME_SIZE];
        copy_string(buf, command, command_length);
        buf[command_length] = '\0';
        fname = buf;
    }


    /*  2. Executable Check   --- WORKS
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

    int8_t ex_buf[EXEC_CHECK_CHARS];
    if (read_data(dentry.inode_num, 0, ex_buf, EXEC_CHECK_CHARS) < EXEC_CHECK_CHARS) {
        sti();
        return ERROR;
    }

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

    int32_t process_num = find_new_process();

    /*
      3. Paging
          - each process gets its own 4 MB page */
    uint32_t phys_addr = ADDR_8MB + (process_num * ADDR_4MB);
    page_dir_init(VIRTUAL_ADDRESS, phys_addr);


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
    pcb_processes[process_num] = pcb_new;
    pcb_processes[process_num]->in_use = 1;
    int index = 0;
    int parent_flag = 0;
    for (; index < NUM_OF_PROCESSES; index++){
        if (pcb_processes[index] != 0x0 && index != process_num){
            if (pcb_processes[index]->in_use == 1){
                pcb_new->parent_pcb = (int32_t)pcb_processes[index];
                pcb_processes[index]->in_use = 0;
                parent_flag = 1;
              }
          }

    }


    /* Copy arguments into pcb */
    if (space_flag == 1){
      copy_string(pcb_new->args, args, arg_buf_len);

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

    //tss.esp0 = pcb_new->mem_addr_start + FOUR_BYTE_ADDR; //8MB - 8KB(process#) - 4 (address length)
    tss.esp0 = ADDR_8MB - ADDR_8KB*(process_num) - FOUR_BYTE_ADDR;
    tss.ss0 = KERNEL_DS; //kernel stack segment = kernel_DS

    pcb_processes[process_num]->esp0 = tss.esp0;
    pcb_processes[process_num]->ss0 = tss.ss0;

    uint32_t curr_ebp;
    asm volatile("                           \n\
                  movl %%ebp, %0             \n\
                  "
                  : "=r"(curr_ebp)
                );
  pcb_new->ebp = curr_ebp;

  //order should be correct. Only moving into ax instead of eax. instead of esp, should be
  //the user stack pointer variable which is the sum of the virtual address and page size,
  //minus the four bits. We also pop somthing from the stack, or it with 0x200 and push it back
  //(not sure why) then push hex 23 and the entry point variable. iret cause iret. ret cause
  //it needs the return address that execute has. Call END_OF_EXECUTE in halt

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
                    addl $20, %%esp         \n\
                    END_OF_EXECUTE:         \n\
                    leave                   \n\
                    ret                     \n\
                    "
                    :                 // (no) ouput
                    : "r"(entry_point), "r"(user_stack_pointer)
                    : "eax","edx"    // clobbered register
                  );
    return SUCCESS; //shouldn't get this far
}

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    /* Check for invalid index */
    int i;
    for (i = 0 ; i < NUM_OF_PROCESSES; i++){
        if (pcb_processes[i] != 0x0){
          if (pcb_processes[i]->in_use == 1) pcb = pcb_processes[i];
        }
    }
    //return Terminal_Read(0,buf, nbytes);
    if (fd < 0 || fd >= FILE_ARRAY_SIZE)
        return ERROR;
    /* Check file position */
    fd_t file_desc = pcb->file_array[fd];
    inode_t* inode = (inode_t*)(boot_block + file_desc.inode);
    /* Return 0 if we've reached the end of the file */


    if (file_desc.file_pos >= inode->length && fd != 0)
        return 0;
    /* Make the correct read call for file type */
    return file_desc.file_ops_table_ptr.read(fd, buf, nbytes);
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {

  int i;
  for (i =0 ; i < NUM_OF_PROCESSES; i++){
      if (pcb_processes[i] != 0x0){
        if (pcb_processes[i]->in_use == 1) pcb = pcb_processes[i];
      }
  }
  // Terminal_Read(0,buf, nbytes);
  if (fd < 0 || fd >= FILE_ARRAY_SIZE)
      return ERROR;
  /* Check file position */
  fd_t file_desc = pcb->file_array[fd];
  /* Return 0 if we've reached the end of the file */
  /* Make the correct read call for file type */
  return file_desc.file_ops_table_ptr.write(fd, buf, nbytes);
}

int32_t open(const uint8_t* filename) {
    dentry_t dentry;
    int i;
    for (i =0 ; i < NUM_OF_PROCESSES; i++){
        if (pcb_processes[i] != 0x0){
          if (pcb_processes[i]->in_use == 1) pcb = pcb_processes[i];
        }
    }
    if (read_dentry_by_name(filename, &dentry) < 0) /* causes warning here?? */
        return ERROR;

    int fa_index;
    int32_t fd;
    for (fa_index = DYNAMIC_FILE_START; fa_index < FILENAME_SIZE; fa_index++) {
        /* Find the next open location in file array */
        fd_t file_desc = pcb->file_array[fa_index];
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
            pcb->file_array[fa_index] = file_desc;
            return fa_index;
        }
    }
    /* There are no open locations in the file array */
    return ERROR;
}

int32_t close(int32_t fd) {
    if (fd < DYNAMIC_FILE_START || fd >= FILE_ARRAY_SIZE)
        return ERROR;
    int i;
    for (i =0 ; i < NUM_OF_PROCESSES; i++){
        if (pcb_processes[i] != 0x0){
          if (pcb_processes[i]->in_use == 1) pcb = pcb_processes[i];

        }
    }
    pcb->file_array[fd].flags = 0;
    return pcb->file_array[fd].file_ops_table_ptr.close(fd);
    return SUCCESS;
}

int32_t getargs(uint8_t* buf, int32_t nbytes) {
    return ERROR;
}

int32_t vidmap(uint8_t** screen_start) {
    return ERROR;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    return ERROR;
}

int32_t sigreturn(void) {
    return ERROR;
}
