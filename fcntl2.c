#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    const char *filename = "example.txt";
    int fd;

    // Open (or create) the file with write permissions
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Prepare a write lock on the entire file
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;   // Write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;        // Start of the lock
    lock.l_len = 0;          // 0 means to lock to EOF

    // Attempt to acquire the lock
    printf("Attempting to acquire write lock...\n");
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl - lock");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Write lock acquired.\n");

    // Write some data to the file
    const char *message = "This is a locked message.\n";
    if (write(fd, message, strlen(message)) == -1) {
        perror("write");
        // Even if write fails, attempt to unlock before exiting
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Data written to the file.\n");

    // Sleep for demonstration purposes (e.g., to keep the lock held)
    printf("Holding lock for 10 seconds...\n");
    sleep(10);

    // Unlock the file
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl - unlock");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Lock released.\n");

    close(fd);
    return 0;
}
