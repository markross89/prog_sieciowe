/*
Serwer oparty na kolejkach FIFO
*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<signal.h>
#include<string.h>

static const char* katalog = "./marekr-FIFO-TMP-SERWER";
int f1, f2;

void ala(int i) {
    close(f1);
    close(f2);
    unlink(katalog);
    exit(0);
}

int main() {
    double a, b;
    char bufor[1000], * pom = bufor;
    pid_t nr_proc, pid;

    signal(SIGTERM, ala);
    signal(SIGINT, ala);
    sigignore(SIGCHLD);
    unlink(katalog);

    if (mkfifo(katalog, 0600) == -1) {
        perror("Serwer: Blad tworzenia kolejki FIFO. [1]\n");
        return -1;
    }
 
    for (;;) {

        if ((f1 = open(katalog, O_RDONLY)) == -1) {
            perror("Serwer: Blad otwarcia kolejki FIFO. [2]\n");
            return -1;
        }

        read(f1, &a, sizeof(double));
        close(f1);

        if ((f2 = open(katalog, O_WRONLY)) == -1) {
            perror("Serwer: Blad otwarcia kolejki FIFO. [2]\n");
            return -1;
        }

        b = a * a;
        write(f2, &b, sizeof(double));
        close(f2);
    }
}
