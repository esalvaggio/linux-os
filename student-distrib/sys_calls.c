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


fotp_t file_funcs = {file_open, file_close, file_read, file_write};
fotp_t rtc_funcs = {RTC_open, RTC_close, RTC_read, RTC_write};
fotp_t dir_funcs = {dir_open, dir_close, dir_read, dir_write};
int8_t executable_check[EXEC_CHECK_CHARS] = {DELETE_CHAR, E_CHAR, L_CHAR, F_CHAR};
// first 4 bytes (0x7f, 0x45, 0x4c, 0x46)

pcb_t* create_new_pcb() {
    int i;
    for (i = 0; i < NUM_OF_PROCESSES; i++) {
        if (pcb_processes[i].in_use == 0) {
          /* Found an open pcb */
          break;
        }
        /* Reached last index and nothing's open */
        else if (i == NUM_OF_PROCESSES - 1)
          return NULL;
    }

    /* Starts at address: 8MB - (process_num * 8KB) */
    pcb_t* new_pcb = (pcb_t*)(ADDR_8MB - (i+1)*ADDR_8KB);
    new_pcb->process_num = i;
    new_pcb->mem_addr_start = ADDR_8MB - (i+1)*ADDR_8KB;

    int fa_index;
    for (fa_index = 2; fa_index < FILE_ARRAY_SIZE; fa_index++) {
      /* Initialize each location in file array to unused */
      new_pcb->file_array[fa_index].flags = 0;
      new_pcb->file_array[fa_index].inode = -1;
      new_pcb->file_array[fa_index].file_pos = 0;
    }

    // set terminal read/write to indexes 0 and 1 here!

    return new_pcb;
}

// void set

int32_t halt(uint8_t status) {

    /*
      1. Restore parent data
        - parent process number (most important)
          -> look in PCB for this
      2. Restore parent paging
        - similar to how we set up paging in execute()
        - flush TLB!
      3. Clear all file descriptors
        - calling close()
      4. Jump back to parent process

    */

    return -1;
}

int32_t execute(const uint8_t* command) {


    /* Instructions
      1. Parse  --- WORKS
          - command: ["filename" + " " + "string of args"]
    */
    int32_t command_idx, arg_buf_len;
    int32_t command_length = strlen((int8_t*)command);
    uint8_t* fname;
    uint8_t* args;
    for (command_idx = 0; command_idx < command_length; command_idx++) {

        if (command[command_idx] == ' ') {
            /* Get filename */
            uint8_t buf[FILENAME_SIZE];
            copy_string(buf, command, command_idx);
            buf[command_idx] = '\0';
            fname = buf;
            printf("%s ", fname);

            /* Get arguments */
            arg_buf_len = command_length - (command_idx + 1);
            uint8_t arg_buf[arg_buf_len];
            copy_string(arg_buf, &command[command_idx + 1], arg_buf_len);
            arg_buf[arg_buf_len] = '\0';
            args = arg_buf;
            printf("%s\n", args);
            break;
        }
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
    if (read_dentry_by_name(fname, &dentry) < 0)
          return -1;

    int8_t ex_buf[4];
    if (read_data(dentry.inode_num, 0, ex_buf, 4) < 0)
        return -1;

    for (command_idx = 0; command_idx < 4; command_idx++) {
        if (ex_buf[command_idx] != executable_check[command_idx])
            return -1;
    }

    /* get entry point from bytes 24-27 */
    if (read_data(dentry.inode_num, 24, ex_buf, 27) < 0)
        return -1;

    /* Adress of first instruction */
    uint32_t entry_point = *((uint32_t*)ex_buf);

    pcb_t* pcb_new = create_new_pcb();
    if (pcb_new == NULL)
        return -1;

    /*
      3. Paging
          - each process gets its own 4 MB page */
    uint32_t phys_addr = ADDR_8MB + (pcb_new->process_num * ADDR_4MB);
    page_dir_init(0x08000000, phys_addr);
          // page_dir_init(0x08000000, 0x0800000); //8mb phys addr user level shell
          // page_dir_init(0x08000000, 0x0C00000); //12mb stuff


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
    if (read_data(dentry.inode_num, 0, (int8_t*)FILE_ENTRY, inode->length) < 0)
        return -1;

    /* Flush TLB by writing to CR3 */
    asm volatile(
                  "movl %%cr3, %%eax;"
                  "movl %%eax, %%cr3;"
                  : /* no outputs */
                  : /* no inputs */
                  : "%eax"
                  );

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


    /* Copy arguments into pcb */
    copy_string(pcb_new->args, args, arg_buf_len);


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

    tss.esp0 = 0x400000; //kernel stack


//push USER_DS, ESP, EFLAG, CS, EIP
    // asm volatile ("                         \n\
    //                 movl %0, %%EAX          \n\
    //                 pushl %%EAX             \n\
    //                 pushl %%ESP             \n\
    //                 pushFL                  \n\
    //                 movl %%eax, %%cr4       \n\
    //                 movl %%cr0, %%eax       \n\
    //                 orl  $0x80000001, %%eax \n\
    //                 movl %%eax, %%cr0       \n\
    //                 "
    //               :             // (no) ouput
    //               : "r"(USER_DS), "r"(csReg)   // page_directory as input. r means go through a regster
    //               : "eax"    // clobbered register
    //               );

  //push USER_DS, ESP, EFLAG, USER_CS, EIP
    asm volatile ("                         \n\
                    movl $0x2B, %%EAX       \n\
                    movl %%EAX, %%DS        \n\
                    pushl %%EAX             \n\
                    pushl %%ESP             \n\
                    pushFL                  \n\
                    movl $0x23, %%EAX       \n\
                    pushl %%EAX             \n\
                    movl %0, %%EAX          \n\
                    pushl %%EAX             \n\
                    "
                    :             // (no) ouput
                    : "r"(entry_point)   // page_directory as input. r means go through a regster
                    : "eax"    // clobbered register
                  );



    return -1;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    /* Check for invalid index */
    if (fd < 0 || fd >= FILE_ARRAY_SIZE)
        return -1;

    /* Check file position */
    fd_t file_desc = pcb->file_array[fd];
    inode_t* inode = (inode_t*)(boot_block + file_desc.inode + 1);
    /* Return 0 if we've reached the end of the file */
    if (file_desc.file_pos >= inode->length)
        return 0;

    /* Make the correct read call for file type */
    return file_desc.file_ops_table_ptr.read(fd, buf, nbytes);
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

int32_t open(const uint8_t* filename) {
    dentry_t dentry;
    if (read_dentry_by_name(filename, &dentry) < 0) /* causes warning here?? */
        return -1;

    int fa_index;
    for (fa_index = 2; fa_index < FILENAME_SIZE; fa_index++) {
        /* Find the next open location in file array */
        fd_t file_desc = pcb->file_array[fa_index];
        if (file_desc.flags != 1) {
            /* Inode is 0 for non-data files */
            if (dentry.filetype == 2) {
                file_desc.inode = dentry.inode_num;
            } else {
                file_desc.inode = 0;
            }
            /* Gets updated every time read is called */
            file_desc.file_pos = 0;
            file_desc.flags = 1;

            int32_t fd;

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

            return fa_index;
        }
    }
    /* There are no open locations in the file array */
    return -1;
}

int32_t close(int32_t fd) {
    if (fd < 2 || fd >= FILE_ARRAY_SIZE)
        return -1;

    pcb->file_array[fd].flags = 0;
    return 0;
}

int32_t getargs(uint8_t* buf, int32_t nbytes) {
    return -1;
}

int32_t vidmap(uint8_t** screen_start) {
    return -1;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

int32_t sigreturn(void) {
    return -1;
}
