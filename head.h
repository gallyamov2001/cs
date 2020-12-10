#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_FIFO "/tmp/seqnum_sv"
/* Well-known name for server's FIFO */
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
/* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
/* Space required for client FIFO pathname
   (+20 as a generous allowance for the PID) */

struct MY_REQUEST {                /* Request (client --> server) */
    pid_t pid;
    char filename[256];
};

const int SIZEOFCHAR = sizeof(char);
const int SIZEOFPID_T = sizeof(pid_t);

void Close_fd(int fd, const char* msg){
    if (close(fd) == -1){
        perror(msg) ;
        exit(1);
    }
}

int Open_fd(const char* name, int flags, const char* msg){

    int Fd = open(name, flags);
    if (Fd == -1){
        perror(msg);
        exit(1);
    }
    return Fd;
}

void DisableNONBLOCK(int fd){
    int flags;
    flags = fcntl(fd, F_GETFL);
    flags &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}
