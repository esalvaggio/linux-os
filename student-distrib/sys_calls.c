#include "sys_calls.h"

#define RTC_FILETYPE     0
#define DIR_FILETYPE     1
#define REG_FILETYPE     2

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

            return 0;
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
