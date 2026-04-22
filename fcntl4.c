/* a very interesting error:

headers with redefinition of ‘struct flock’

*/



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>     //for error this commented
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <linux/fcntl.h>  //for error this uncommented

// /* instead of #include <linux/fcntl.h>   
#define F_GETSIG  11
#define F_SETSIG  10
#define F_SETLEASE  1024
#define F_GETLEASE  1025
#define F_NOTIFY  9
#define F_SETPIPE_SZ  1031
#define F_GETPIPE_SZ  1032
#define F_ADD_SEALS  1033
#define F_GET_SEALS  1034
#define F_GET_RW_HINT  1035
#define F_SET_RW_HINT  1036
//*/                           //for error this commented



int main() {
    const char *filename = "testfile.txt";

    // 1. Create and open a file
    int fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("Opened file '%s' with fd=%d", filename, fd);

    // 2. Duplicate fd using F_DUPFD
    int dup_fd = fcntl(fd, F_DUPFD, 0);
    if (dup_fd == -1) {
        perror("F_DUPFD");
    } else {
        printf("Duplicated fd=%d to fd=%d", fd, dup_fd);
    }

    // 3. Duplicate fd with close-on-exec flag using F_DUPFD_CLOEXEC
    int dup_cloexec_fd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (dup_cloexec_fd == -1) {
        perror("F_DUPFD_CLOEXEC");
    } else {
        printf("Duplicated with CLOEXEC fd=%d", dup_cloexec_fd);
    }

    // 4. Get and set FD flags (like FD_CLOEXEC)
    int fd_flags = fcntl(fd, F_GETFD);
    if (fd_flags == -1) {
        perror("F_GETFD");
    } else {
        printf("FD flags before: 0x%x", fd_flags);
        // Set FD_CLOEXEC
        if (fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC) == -1) {
            perror("F_SETFD");
        } else {
            printf("Set FD_CLOEXEC flag");
        }
    }

    // 5. Get and set file status flags (like O_NONBLOCK)
    int status_flags = fcntl(fd, F_GETFL);
    if (status_flags == -1) {
        perror("F_GETFL");
    } else {
        printf("File status flags before: 0x%x", status_flags);
        // Set O_NONBLOCK
        if (fcntl(fd, F_SETFL, status_flags | O_NONBLOCK) == -1) {
            perror("F_SETFL");
        } else {
            printf("Set O_NONBLOCK flag");
        }
    }

    // 6. Advisory record locking (lock the first 100 bytes)
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;   // Write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 100;

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("F_SETLK");
    } else {
        printf("Acquired advisory write lock on first 100 bytes");
    }

    // 7. Get lock info
    struct flock get_lock;
    memset(&get_lock, 0, sizeof(get_lock));
    get_lock.l_type = F_WRLCK;
    get_lock.l_whence = SEEK_SET;
    get_lock.l_start = 0;
    get_lock.l_len = 100;

    if (fcntl(fd, F_GETLK, &get_lock) == -1) {
        perror("F_GETLK");
    } else {
        printf("Lock info: type=%d, start=%lld, len=%lld, pid=%d",
               get_lock.l_type, (long long)get_lock.l_start, (long long)get_lock.l_len, get_lock.l_pid);
    }

    // 8. Duplicate using F_DUPFD for open file description lock (non-POSIX)
    // Note: F_OFD_SETLK etc. are Linux-specific; ensure your kernel supports them.
    // For demonstration, we skip actual lock setting with F_OFD_SETLK if not supported.

    // 9. Managing signals: get and set owner
    pid_t pid = getpid();
    if (fcntl(fd, F_GETOWN) == -1) {
        perror("F_GETOWN");
    } else {
        printf("Current owner: %d", fcntl(fd, F_GETOWN));
    }

    if (fcntl(fd, F_SETOWN, pid) == -1) {
        perror("F_SETOWN");
    } else {
        printf("Set owner to PID %d", pid);
    }

    // 10. Get and set signal
    int sig = 10; // SIGUSR1
    if (fcntl(fd, F_GETSIG) == -1) {
        perror("F_GETSIG");
    } else {
        printf("Current signal: %d", fcntl(fd, F_GETSIG));
    }
    if (fcntl(fd, F_SETSIG, sig) == -1) {
        perror("F_SETSIG");
    } else {
        printf("Set signal to %d", sig);
    }

    // 11. File lease (requires kernel support)
    // This part may not work on all systems
    int lease_type = F_WRLCK;
    if (fcntl(fd, F_SETLEASE, lease_type) == -1) {
        perror("F_SETLEASE");
    } else {
        printf("Lease set to write lock");
    }

    // 12. Get lease
    int lease;
    if (fcntl(fd, F_GETLEASE, &lease) == -1) {
        perror("F_GETLEASE");
    } else {
        printf("Lease: %d", lease);
    }

    // 13. Change notification (dnotify)
    int notify_mask = 0; // For example, 0 disables notifications
    if (fcntl(fd, F_NOTIFY, notify_mask) == -1) {
        perror("F_NOTIFY");
    } else {
        printf("Changed notification settings");
    }

    // 14. Change pipe capacity
    int pipe_size = 1024;
    int new_size = fcntl(fd, F_SETPIPE_SZ, pipe_size);
    if (new_size == -1) {
        perror("F_SETPIPE_SZ");
    } else {
        printf("Set pipe size to %d bytes", new_size);
    }
    int current_size = fcntl(fd, F_GETPIPE_SZ);
    if (current_size == -1) {
        perror("F_GETPIPE_SZ");
    } else {
        printf("Current pipe size: %d bytes", current_size);
    }

    // 15. File sealing (requires system support, may not work in all environments)
    int seals = 0;
    if (fcntl(fd, F_ADD_SEALS, seals) == -1) {
        perror("F_ADD_SEALS");
    } else {
        printf("Added seals (value: %d)\n", seals);
    }
    int get_seals;
    if (fcntl(fd, F_GET_SEALS, &get_seals) == -1) {
        perror("F_GET_SEALS");
    } else {
        printf("Seals: %d", get_seals);
    }

    // 16. Read/write hints (not portable; Linux-specific)
    int rw_hint = 0;
    if (fcntl(fd, F_GET_RW_HINT, &rw_hint) == -1) {
        perror("F_GET_RW_HINT");
    } else {
        printf("Read/write hint: %d", rw_hint);
    }
    int new_hint = 1;
    if (fcntl(fd, F_SET_RW_HINT, new_hint) == -1) {
        perror("F_SET_RW_HINT");
    } else {
        printf("Set read/write hint to %d", new_hint);
    }

    // Cleanup
    close(fd);
    close(dup_fd);
    close(dup_cloexec_fd);

    // Optionally, delete the test file
    unlink(filename);

    return 0;
}
