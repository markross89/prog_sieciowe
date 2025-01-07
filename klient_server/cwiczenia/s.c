#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

int fd_r, fd_w, fd_s;
char* fifo_s = "fifo_serwer";

void obsluga(int s)
{
    close(fd_r);
    close(fd_w);
    close(fd_s);
    unlink(fifo_s);
    printf("\npamiec wyczyszczona\n");
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGTERM, obsluga);
    signal(SIGINT, obsluga);

    unlink(fifo_s);
    if (mkfifo(fifo_s, 0600) == -1)
    {
        perror("serwer: mkfifo");
        return -1;
    }

    printf("Serwer...\n");

    // otwarcie fifo spowoduje zablokowanie procesu do momentu otwarcia tego samego pliku w kliencie
    // w serwerze fifo musi byc otwarte do czytania i zapisu (zeby oproznic fifo za kazdym obrotem petli),
    // chociaz nie korzystamy z zapisu w przeciwnym wypadku serwer zapetli sie czytajac w kolko ten same dane
    if ((fd_s = open(fifo_s, O_RDWR)) == -1)
    {
        perror("serwer: open");
        unlink(fifo_s);
        return -1;
    }

    for (;;)
    {
        pid_t pid;
        read(fd_s, &pid, sizeof(pid_t));
        printf("serwer: otrzymalem wiadomosc od %d\n", pid);
        
        char fifo_w[100];
        sprintf(fifo_w, "fifo_r_%d", pid);
        if ((fd_w = open(fifo_w, O_WRONLY)) == -1)
        {
            perror("serwer: open");
            close(fd_s);
            unlink(fifo_s);
            return -1;
        }

        char* m = "serwer: prosze podaj liczbe";
        write(fd_w, m, sizeof(*m) * strlen(m));

        char fifo_r[100];
        sprintf(fifo_r, "fifo_w_%d", pid);
        if ((fd_r = open(fifo_r, O_RDONLY)) == -1)
        {
            perror("serwer: open");
            close(fd_s);
            close(fd_w);
            unlink(fifo_s);
            return -1;
        }

        int liczba;
        read(fd_r, &liczba, sizeof(int));
        printf("serwer: otrzymalem liczbe %d\n", liczba);

        liczba *= liczba;
        write(fd_w, &liczba, sizeof(liczba));
        printf("serwer: podnioslem %d do kwadratu i przeslalem wynik do %d\n", liczba, pid);
    }
}