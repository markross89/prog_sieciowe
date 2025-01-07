#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

int main()
{
    char* fifo_s = "fifo_serwer";
    char fifo_r[50];
    char fifo_w[50];
    int fd_w, fd_r, fd_s;

    pid_t pid = getpid();

    sprintf(fifo_r, "fifo_r_%d", pid);
    if (mkfifo(fifo_r, 0600) == -1)
    {
        perror("serwer: mkfifo[1]");
        return -1;
    }

    sprintf(fifo_w, "fifo_w_%d", pid);
    if (mkfifo(fifo_w, 0600) == -1)
    {
        perror("serwer: mkfifo[2]");
        unlink(fifo_r);
        return -1;
    }

    printf("Klient...\n");
    if ((fd_s = open(fifo_s, O_WRONLY)) == -1)
    {
        perror("Klient: open[1]");
        unlink(fifo_w);
        unlink(fifo_r);
        return -1;
    }

    write(fd_s, &pid, sizeof(pid_t));
    close(fd_s);

    if ((fd_r = open(fifo_r, O_RDONLY)) == -1)
    {
        perror("Klient: open[2]");
        unlink(fifo_r);
        unlink(fifo_w);
        return -1;
    }
    char m[100];
    read(fd_r, &m, sizeof(char) * 100);
    printf("%s\n", m);

    if ((fd_w = open(fifo_w, O_WRONLY)) == -1)
    {
        perror("Klient: open[3]");
        close(fd_r);
        unlink(fifo_w);
        unlink(fifo_r);
        return -1;
    }

    int liczba;
    scanf("%d", &liczba);
    write(fd_w, &liczba, sizeof(int));

    int wynik;
    read(fd_r, &wynik, sizeof(int));
    printf("klient: serwer wyliczyl %d^2 =  %d\n", liczba, wynik);

    close(fd_r);
    close(fd_w);
    unlink(fifo_r);
    unlink(fifo_w);
    return 0;
}