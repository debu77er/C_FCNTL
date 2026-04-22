#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define LOCK_START 0
#define LOCK_LEN 100

void print_lock(struct flock *lock) {
    printf("Lock type: %s", lock->l_type == F_WRLCK ? "Write" :
                                lock->l_type == F_RDLCK ? "Read" :
                                lock->l_type == F_UNLCK ? "Unlock" : "Unknown");
    printf("Start: %lld, len: %lld", (long long)lock->l_start, (long long)lock->l_len);
    printf("PID: %d", lock->l_pid);
}

int main(int argc, char *argv[]) {
    const char *filename = "testfile.txt";
    int fd;

    // Create or open the file
    fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write some data to the file
    const char *data = "This is a test file for fcntl locking.\n";
    write(fd, data, strlen(data));

    // Initialize a flock structure to set a lock
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK; // try to acquire a write lock
    lock.l_start = LOCK_START;
    lock.l_whence = SEEK_SET;
    lock.l_len = LOCK_LEN;

    // Check if the region is locked by someone else
    struct flock get_lock = lock;
    get_lock.l_type = F_WRLCK;
    if (fcntl(fd, F_GETLK, &get_lock) == -1) {
        perror("fcntl F_GETLK");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (get_lock.l_type == F_UNLCK) {
        printf("Region is free, acquiring lock...\n");
        if (fcntl(fd, F_SETLK, &lock) == -1) {
            perror("fcntl F_SETLK");
            close(fd);
            exit(EXIT_FAILURE);
        } else {
            printf("Lock acquired.\n");
        }
    } else {
        printf("Region is already locked by process %d.\n", get_lock.l_pid);
        printf("Existing lock details:\n");
        print_lock(&get_lock);
        // Optionally, you can try to acquire the lock forcibly with F_SETLKW
    }

    // Simulate some operation
    printf("Holding lock for 5 seconds...\n");
    sleep(5);

    // Release the lock
    struct flock unlock;
    memset(&unlock, 0, sizeof(unlock));
    unlock.l_type = F_UNLCK;
    unlock.l_start = LOCK_START;
    unlock.l_whence = SEEK_SET;
    unlock.l_len = LOCK_LEN;

    if (fcntl(fd, F_SETLK, &unlock) == -1) {
        perror("fcntl to unlock");
        close(fd);
        exit(EXIT_FAILURE);
    } else {
        printf("Lock released.\n");
    }

    close(fd);
    return 0;
}
