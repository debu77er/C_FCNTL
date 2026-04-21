#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
    const char *filename = "example.txt";

    // Open the file for reading and writing, create if it doesn't exist
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Get current file status flags
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        perror("Failed to get file status flags");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Current flags: 0x%x", flags);

    // Get current access mode
    int access_mode = flags & O_ACCMODE;
    printf("Current access mode: ");
    if (access_mode == O_RDONLY)
        printf("Read only");
    else if (access_mode == O_WRONLY)
        printf("Write only");
    else if (access_mode == O_RDWR)
        printf("Read/write");
    else
        printf("Unknown");

    // Add O_APPEND flag to the current flags
    int new_flags = flags | O_APPEND;
    if (fcntl(fd, F_SETFL, new_flags) == -1) {
        perror("Failed to set new flags");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("O_APPEND flag set. New flags: 0x%x", new_flags);

    // Verify the change
    int verify_flags = fcntl(fd, F_GETFL);
    if (verify_flags == -1) {
        perror("Failed to verify flags");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Verified flags: 0x%x", verify_flags);

    // Write some data to the file
    const char *data = "Hello, fcntl!\n";
    ssize_t bytes_written = write(fd, data, strlen(data));
    if (bytes_written == -1) {
        perror("Failed to write to file");
    } else {
        printf("Wrote %zd bytes to the file.\n", bytes_written);
    }

    // Close the file
    close(fd);
    return 0;
}
