#include "fs_setup.h"
#include "lib.h"

void fs_init(uint32_t module_addr) {
    boot_block = ((boot_block_t*)(module_addr));
}

int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry) {

    int32_t entry_index;
    dentry_t dir_entry;

    for (entry_index = 0; entry_index < boot_block->dir_count; entry_index++) {

        dir_entry = boot_block->d_entries[entry_index];

        /* Check if the filenames match */
        if (!strncmp(fname, dir_entry.filename, FILENAME_SIZE)) {
            /* Call the function below to set values */
            return read_dentry_by_index(entry_index, dentry);
        }

    }

    return -1;
}

int32_t read_dentry_by_index(int32_t index, dentry_t* dentry) {

    /* Check for invalid index */
    if (index < 0 || index >= boot_block->dir_count)
        return -1;

    dentry_t dir_entry;
    dir_entry = boot_block->d_entries[index];

    /* Copy filename, filetype and inode to the entry */
    copy_string(dentry->filename, dir_entry.filename, FILENAME_SIZE);
    dentry->filetype  = dir_entry.filetype;
    dentry->inode_num = dir_entry.inode_num;
    return 0;

}

int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length) {

    /* Check for invalid inode */
    if (inode < 0 || inode >= boot_block->inode_count)
        return -1;

    /* Check if we've reached the end of the file */
    if (offset >= length)
        return 0;

    /* Calculate how many data blocks are used */
    inode_t* inode_block = (inode_t*)(boot_block + inode + 1);
    int32_t num_data_blocks = inode_block->length/BLOCK_SIZE;

    /* We can only read up to the length of the file itself */
    if (length > inode_block->length)
        length = inode_block->length;

    int32_t db_index, bytes_read = 0;

    /* Read from data block */
    for (db_index = 0; db_index < num_data_blocks; db_index++) {

        /* Keeps track of index within a data_block*/
        int32_t data_index = 0;
        uint8_t* data_block = (uint8_t*)(boot_block + boot_block->inode_count + db_index + 1);

        /* Determines when we need to go to the next data block */
        while (bytes_read < BLOCK_SIZE) {
            /* Read up to "length - offset" bytes */
            if (bytes_read >= length - offset)
                return bytes_read;

            if (db_index != 0) {
                buf[bytes_read] = data_block[data_index];
                data_index++;
            } else {
                buf[bytes_read] = data_block[bytes_read + offset];
            }

            bytes_read++;
        }
    }

    return bytes_read;
}

void copy_string(int8_t* dest, const int8_t* src, uint32_t n) {
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
