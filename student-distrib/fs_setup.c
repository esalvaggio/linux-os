#include "fs_setup.h"
#include "sys_calls.h"
#include "lib.h"

/* fs_init
 *
 * Initializes the boot block starting address
 * Inputs:
 *    module_addr - starting address of boot block
 * Outputs: None
 * Side Effects: Initializes our boot_block variable
 *
 */
void fs_init(uint32_t module_addr) {
    boot_block = ((boot_block_t*)(module_addr));
}

/* read_dentry_by_name
 *
 * Searches through the directory entries and tries to
 *	match the filename input with an entry that has that
 *	same file name.
 * Inputs:
 *    fname - filename to search for
 *    dentry - the directory entry we are setting
 * Outputs: 0 on success, -1 on fail
 * Side Effects: Calls read_dentry_by_index
 *
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {

    int32_t entry_index;
    dentry_t dir_entry;

    if (strlen((int8_t*)fname) > FILENAME_SIZE)
        return -1;

    for (entry_index = 0; entry_index < boot_block->dir_count; entry_index++) {
        /* Obtain the directory entry */
        dir_entry = boot_block->d_entries[entry_index];
        /* Check if the filenames match */
        if (!strncmp((int8_t*)fname, (int8_t*)dir_entry.filename, FILENAME_SIZE)) {
            /* Call the function below to set values */
            return read_dentry_by_index(entry_index, dentry);
        }
    }

    return -1;
}

/* read_dentry_by_index
 *
 * Searches through the directory entries and tries to
 *	match the index input with the specified index in
 * our dentry array.
 * Inputs:
 *    index - index of directory entry
 *    dentry - the directory entry we are setting
 * Outputs: 0 on success, -1 on fail
 * Side Effects: Sets the filename, filetype, and inode
 *  of the dentry argument
 *
 */
int32_t read_dentry_by_index(int32_t index, dentry_t* dentry) {
    /* Check for invalid index */
    if (index < 0 || index >= boot_block->dir_count)
        return -1;

    dentry_t dir_entry = boot_block->d_entries[index];

    /* Copy filename, filetype and inode to the entry */
    copy_string(dentry->filename, dir_entry.filename, FILENAME_SIZE);
    dentry->filetype  = dir_entry.filetype;
    dentry->inode_num = dir_entry.inode_num;
    return 0;

}

/* read_data
 *
 * Reads the data of the file from the specific data
 *	blocks. The inode number is used to find which data
 *	blocks we need to read from.
 * Inputs:
 *    inode - inode number from the directory entry
 *    offset - where to start reading data
 *    buf - the buffer that will store the data
 *    length - where to end reading data
 * Outputs: 0 on success, -1 on fail
 * Side Effects: Fills the buf argument with the data we
 *  are trying to read from.
 *
 */
int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length) {
    /* Check for invalid inode */
    if (inode < 0 || inode >= boot_block->inode_count)
        return -1;

    inode_t* inode_block = (inode_t*)(boot_block + inode + 1);
    /* Check if we've reached the end of the file */
    if (offset >= inode_block->length)
        return 0;

    if (offset + length >= inode_block->length)
        length = inode_block->length - offset;

    /* Calculate how many data blocks are used */
    int32_t num_data_blocks = (inode_block->length/BLOCK_SIZE) + 1;
    // /* We can only read up to the length of the file itself */

    int32_t block_index, bytes_read = 0;
    int32_t data_index = offset % BLOCK_SIZE;
    /* Read from data block */
    for (block_index = offset/BLOCK_SIZE; block_index < num_data_blocks; block_index++) {
        /* Keeps track of index within a data_block*/
        int32_t db_index = inode_block->data_block_num[block_index];
        uint8_t* data_block = (uint8_t*)(boot_block + boot_block->inode_count + db_index + 1);
        /* Determines when we need to go to the next data block */
        while (((bytes_read + offset) % BLOCK_SIZE != 0) || (data_index == 0)) {
            /* Read up to "length" bytes */
            if (bytes_read >= length){
                return bytes_read;
              }
            /* Read data and write it to the buffer */
            buf[bytes_read] = data_block[data_index];
            data_index++;
            bytes_read++;
        }
        /* Reset our index each time we go to a new data_block */
        data_index = 0;
    }

    return bytes_read;
}

/* Global directory entry */
dentry_t dir;

/* file_open
 *
 * "Opens" a file and set the dir variable by calling
 *  read_dentry_by_name.
 * Inputs:
 *    filename - name of the file to open
 * Outputs: 0 on success, -1 on fail
 * Side Effects: None
 *
 */
int32_t file_open(const uint8_t* filename) {
    if (read_dentry_by_name(filename, &dir) < 0)
        return -1;

    return 0;
}

/* file_close
 *
 * Closes a file by undoing the file_open call
 * Inputs:
 *    fd - file descriptor
 * Outputs: 0 on success
 * Side Effects: None
 *
 */
int32_t file_close(int32_t fd) {
    pcb_t* curr_pcb = get_pcb_ptr();
    fd_t* file_desc = &curr_pcb->file_array[fd];
    file_desc->flags = 0;
    file_desc->inode = -1;
    file_desc->file_pos = 0;
    /* Reset the file operations table pointer */
    fotp_t no_fotp = {NULL, NULL, NULL, NULL};
    file_desc->file_ops_table_ptr = no_fotp;
    return 0;
}

/* file_read
 *
 * Read the contents of the file.
 * Inputs:
 *    fd - file descriptor
 *    buf - buffer that stores the data read
 *    nbytes - number of bytes to be read
 * Outputs: the number of bytes read, -1 if fails
 * Side Effects: None
 *
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    /* Find the current file position */
    pcb_t* curr_pcb = get_pcb_ptr();
    fd_t* file_desc = &curr_pcb->file_array[fd];
    /* Read up to "length" */
    int32_t bytes_read = read_data(file_desc->inode, file_desc->file_pos, buf, nbytes);
    /* Update the file position after reading */
    file_desc->file_pos += bytes_read;
    return bytes_read;
}

/* file_write
 *
 * Does nothing because of read-only file system.
 * Inputs:
 *    filename - name of the file to close
 * Outputs: -1 always
 * Side Effects: None
 *
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* dir_open
 *
 * Opens a directory file, similar to file_open.
 * Inputs:
 *    filename - name of the file to open
 * Outputs: 0 on success, -1 on fail
 * Side Effects: None
 *
 */
int32_t dir_open(const uint8_t* filename) {
    if (read_dentry_by_name(filename, &dir) < 0)
        return -1;

    return 0;
}

/* dir_close
 *
 * Closes a file by undoing the dir_open call
 * Inputs:
 *    fd - file descriptor
 * Outputs: 0 on success
 * Side Effects: None
 *
 */
int32_t dir_close(int32_t fd) {
    pcb_t* curr_pcb = get_pcb_ptr();
    fd_t* file_desc = &curr_pcb->file_array[fd];
    file_desc->flags = 0;
    file_desc->inode = -1;
    file_desc->file_pos = 0;
    /* Reset the file operations table pointer */
    fotp_t no_fotp = {NULL, NULL, NULL, NULL};
    file_desc->file_ops_table_ptr = no_fotp;
    return 0;
}

/* dir_read
 *
 * Reads from a directory and prints out the filenames,
 *  filetypes, and sizes of each file from within.
 * Inputs:
 *    fd - file descriptor
 *    buf - buffer that stores the data read
 *    nbytes - number of bytes to read
 * Outputs: number of bytes read from filename, 0 once we are done
 * Side Effects: None
 *
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* curr_pcb = get_pcb_ptr();
    fd_t* file_desc = &curr_pcb->file_array[fd];
    /* We are done reading all the files in the directory */
    if (file_desc->file_pos >= boot_block->dir_count)
        return 0;

    dentry_t dir = boot_block->d_entries[file_desc->file_pos];

    copy_string(buf, dir.filename, nbytes);
    int32_t i = 0;
    while (dir.filename[i] != '\0' && i < nbytes) {
        i++;
    }
    /* Update the file_pos to what file should be read */
    file_desc->file_pos++;
    /* Return the number of bytes read from the filename */
    return i;
}

/* dir_write
 *
 * Does nothing because of the read-only file system.
 * Inputs:
 *    filename - name of the file to close
 * Outputs: -1 always
 * Side Effects: None
 *
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* copy_string
 *
 * Copies n characters from one string into another.
 * Inputs:
 *    dest - string to copy to
 *    src - string to copy from
 *    n - number of bytes of to copy up to
 * Outputs: 0 on success
 * Side Effects: None
 *
 */
void copy_string(uint8_t* dest, const uint8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
}
