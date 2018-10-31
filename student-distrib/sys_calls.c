#include "sys_calls.h"

#define RTC_FILETYPE      0
#define DIR_FILETYPE      1
#define REG_FILETYPE      2
#define DELETE_CHAR    0x7F
#define E_CHAR         0x45
#define L_CHAR         0x4C
#define F_CHAR         0x46


fotp_t file_funcs = {file_open, file_close, file_read, file_write};
fotp_t rtc_funcs = {RTC_open, RTC_close, RTC_read, RTC_write};
fotp_t dir_funcs = {dir_open, dir_close, dir_read, dir_write};
int8_t executable_check[4] = {DELETE_CHAR, E_CHAR, L_CHAR, F_CHAR};
// first 4 bytes (0x7f, 0x45, 0x4c, 0x46)

void pcb_init() {
    int fa_index;
    for (fa_index = 0; fa_index < FILENAME_SIZE; fa_index++) {
      /* Initialize each location in file array to unused */
      pcb->file_array[fa_index].flags = 0;
    }
}

// void set

int32_t halt(uint8_t status) {
    return -1;
}

int32_t execute(const uint8_t* command) {


    /* Instructions
      1. Parse  --- WORKS
          - command: ["filename" + " " + "string of args"]
    */
    int32_t command_idx;
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
            int32_t arg_buf_len = command_length - (command_idx + 1);
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

    /*
      3. Paging
          - each process gets its own 4 MB page
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
      5. create PCB
          - args should be put into PCB
          - allocate space for PCB in memory
          - total kernel stack goes from 4MB - 8MB
            -> kernel stack for process 1 => 8MB -> (8MB - 8KB)
                -> very bottom of kernel stack
            -> kernel stack for process 2 => (8MB - 8KB) -> 8KB
                -> above previous process kernel stack
            -> ... so on
      6. context switch
          - change priviledge level
          - IRET (read in ISA manual)
          - push artificial IRET onto the stack
            -> IRET pops 32B from the stack, so use PUSHL
            -> push the new (user) ESP and segment selector
          - should not be able to leave shell!!
            -> first user-level program called in kernel.c
    */
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
