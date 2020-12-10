#include "head.h"

char clientfifo[CLIENT_FIFO_NAME_LEN];


int main(int argc, char *argv[]){


    struct MY_REQUEST req;


    if (argc != 2 ){
        printf("invalid argc\n");
        exit(1);
    }

    umask(0);

    snprintf(clientfifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());
    if (mkfifo(clientfifo, 0666) == -1  && errno != EEXIST){
        perror("mkfifo");
        exit(1);
    }

    /* создаем запрос */
    req.pid = getpid();
    strcpy(req.filename, argv[1]);
    
    
    /* открываем трубу сервера */
    int server_fd = Open_fd(SERVER_FIFO, O_WRONLY | O_NONBLOCK, "");
    DisableNONBLOCK(server_fd);


    /* открываем нашу трубу c нонблоком, затем отключаем этот режим */
    int client_fd = Open_fd(clientfifo, O_RDONLY | O_NONBLOCK, "file receiverFd open");
    DisableNONBLOCK(client_fd);

    /* пишем серверу запрос */
    write(server_fd, &req, sizeof(struct MY_REQUEST));

    char buf[PIPE_BUF] = "";
    int indicator = 0;
    int reallength = PIPE_BUF;

    /* внутренний цикл нужен для того чтобы ждать ответа сервера только определенное время */

    while(reallength == PIPE_BUF && !buf[0] ) {
        int i = 0;
        while(i < 5)  /*  ждем до 5 секунд*/
        {
            ioctl(client_fd, FIONREAD, &indicator);  /* проверить если ли инфа в трубе */
            if (indicator){
                break;
            }
            i++;
            sleep(1);
        }

        if (i == 5){     /* если 5 секунд прошли то ошибка */
            printf("server failed or going too slow\n");
            exit(1);
        }
        reallength = read(client_fd, buf,  PIPE_BUF);
        write(STDOUT_FILENO, buf + 1 , reallength - 1);

    }

    Close_fd(client_fd, "rewriteFd close");
    Close_fd(server_fd, "contactFd close");

}

