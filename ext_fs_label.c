#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: I need a block device path.\n");
        return EXIT_FAILURE;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: open() failed: %m\n");
        return EXIT_FAILURE;
    }
    char *boot_sector = (char *) malloc(1024 * sizeof(char));
    if (read(fd, boot_sector, 1024) < 0) {
        fprintf(stderr, "ERROR: read() failed: %m\n");
        free(boot_sector);
        close(fd);
        return EXIT_FAILURE;
    }
    free(boot_sector);
    struct ext2_super_block super_block;
    bzero(&super_block, sizeof(struct ext2_super_block));
    if (read(fd, &super_block, sizeof(struct ext2_super_block)) < 0) {
        fprintf(stderr, "ERROR: read() failed: %m\n");
        close(fd);
        return EXIT_FAILURE;
    }
    close(fd);
    printf("Label of %s is: %s\n", argv[1], super_block.s_volume_name);
    return EXIT_SUCCESS;
}
