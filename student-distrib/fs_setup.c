#include "fs_setup.h"
#include "lib.h"

void fs_init(uint32_t module_addr) {
    // printf("Boot block address: %d\n", module_addr);
    boot_block = ((boot_block_t*)(module_addr));
}

int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry) {
    printf("we good 1\n");
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

    printf("we good 2\n");

    return -1;
}

int32_t read_dentry_by_index(int32_t index, dentry_t* dentry) {
    printf("we good 3\n");
    /* Check for invalid index */
    if (index < 0 || index >= boot_block->dir_count)
        return -1;

    dentry_t dir_entry = boot_block->d_entries[index];
    printf("we good 3.5\n");
    /* Copy filename, filetype and inode to the entry */
    copy_string(dentry->filename, dir_entry.filename, FILENAME_SIZE);
    printf("we good 3.7\n");
    dentry->filetype  = dir_entry.filetype;
    dentry->inode_num = dir_entry.inode_num;
    printf("%d\n", dentry->filetype);
    printf("%d\n", dentry->inode_num);
    printf("%d\n", boot_block->inode_count);
    printf("we good 4\n");
    return 0;

}

int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length) {
    printf("we good 5\n");
    /* Check for invalid inode */
    if (inode < 0 || inode >= boot_block->inode_count)
        return -1;

    /* Check if we've reached the end of the file */
    if (offset >= length)
        return 0;

    /* Calculate how many data blocks are used */
    inode_t* inode_block = (inode_t*)(boot_block + inode + 1);
    printf("file length: %d\n", inode_block->length);
    int32_t num_data_blocks = (inode_block->length/BLOCK_SIZE) + 1;
    printf("%d\n", num_data_blocks);
    printf("we good 6\n");
    /* We can only read up to the length of the file itself */
    if (length > inode_block->length)
        length = inode_block->length;

    int32_t block_index, bytes_read = 0;
    printf("we good 7\n");
    /* Read from data block */
    for (block_index = 0; block_index < num_data_blocks; block_index++) {

        /* Keeps track of index within a data_block*/
        int32_t data_index = 0, db_index = inode_block->data_block_num[block_index];
        uint8_t* data_block = (uint8_t*)(boot_block + boot_block->inode_count + db_index + 1);

        /* Determines when we need to go to the next data block */
        while (((bytes_read + offset) % BLOCK_SIZE != 0) || (bytes_read + offset == 0)) {
            printf("%d ", bytes_read + offset);
            /* Read up to "length - offset" bytes */
            if (bytes_read >= length - offset){
                printf("str read %s\n", buf);
                return bytes_read;
              }
            if (block_index != 0) {
                buf[bytes_read] = data_block[data_index];
                printf("No! ");
                data_index++;
            } else {
                buf[bytes_read] = data_block[bytes_read + offset];
                // putc(buf[bytes_read]);
            }

            bytes_read++;
        }
    }
    printf("we good 8\n");
    return bytes_read;
}

int32_t file_open(int8_t* filename, dentry_t* dir) {

    if (read_dentry_by_name(filename, dir) < 0)
        return -1;

    printf("%s\n", dir->filename);
    printf("We good 4.2\n");
    return 0;
    /* Need to implement file descriptor */
}

int32_t file_read(int8_t* filename, int8_t* data_buf) {
    dentry_t dir;
    if (file_open(filename, &dir) < 0)
        return -1;

    printf("We good 4.3\n");

    if (read_data(dir.inode_num, 0, data_buf, 187) <= 0)
        return -1;

    printf("We good 4.4\n");
    return 0;
}

int32_t file_write(int8_t* filename) {
    return -1;
}

int32_t dir_open(int8_t* filename, dentry_t* dir) {
    return -1;
}

int32_t dir_read(int8_t* filename) {
    return -1;
}

int32_t dir_write(int8_t* filename) {
    return -1;
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
