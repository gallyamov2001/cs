
#include "head.h"

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    char clientfifo[CLIENT_FIFO_NAME_LEN];
    struct MY_REQUEST req;

    umask(0);
    
    if (mkfifo(SERVER_FIFO, 0666) == -1 && errno != EEXIST){
        perror("mkfifo");
        exit(1);
    }
    
    /* открываем трубу сервера (если клиент откроет ее раньше нас то умрет и это логично)*/
    
    server_fd = Open_fd(SERVER_FIFO, O_RDWR, "");
    /* будем игнорировать то, что клиент упал, т.к. сервер не должен от этого упасть */
    signal(SIGPIPE, SIG_IGN);



    while(1) {
        if (read(server_fd, &req, sizeof(struct MY_REQUEST)) != sizeof(struct MY_REQUEST)) {
            fprintf(stderr, "Error reading request\n");
            continue;
        }

        /* открываем клиентское фифо и файл, который передал клиент */

        snprintf(clientfifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);
        int client_fd = open(clientfifo, O_WRONLY | O_NONBLOCK);
        if (client_fd == -1) {
            perror("CLIENT");
            continue;
        }
        DisableNONBLOCK(client_fd);

        FILE* file_in = fopen(req.filename, "rb");
        if (file_in == NULL) {
            perror("CLIENT");
            continue;
        }

        char buf[PIPE_BUF] = "";
        int reallength = 0;

        while((reallength = fread(buf + 1, SIZEOFCHAR,  PIPE_BUF - 1, file_in)) == PIPE_BUF - 1 ){

            buf[0] = 0;
            if(write(client_fd, buf, PIPE_BUF) == -1){
                perror("CLIENT");
                close(client_fd);
                break;
            }

        }
        
        buf[0] = 1;
        write(client_fd, buf, reallength + 1);

        close(client_fd);           /* закрываем клиентскую трубу, идем к следующему клиенту */
    }
}
