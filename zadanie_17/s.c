/*
Serwer wspolbiezny oparty na kolejekach komunikatow.
*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<mqueue.h>
#include<sys/stat.h>


typedef struct mymesg1 {
    long mtype;
    pid_t pid;
} paczka1;
typedef struct mymesg2 {
    long mtype;
    double melement;
} paczka2;

int fd;
paczka1* wiad1;
char* sciezka;
int czy_skasowac = 1;

void ala(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("Serwer: Koniec pracy serwera.\n");
        mq_close(fd);
        mq_unlink(sciezka);
        free(wiad1);
        free(sciezka);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    pid_t pid;
    const size_t wielkosc1 = sizeof(paczka1);
    ssize_t liczba_bajtow;
    int plik = 0;

    signal(SIGTERM, ala);
    signal(SIGINT, ala);
    sigignore(SIGCHLD);

    if (argc > 1) {
        sciezka = strndup(argv[1], strlen(argv[1]));
    }
    else {
        sciezka = strndup("/kolejka", strlen("/kolejka"));
    }

    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 5, .mq_msgsize = 100 };

    mq_unlink(sciezka);
    if ((fd = mq_open(sciezka, O_CREAT | O_RDONLY, 0600, &attr)) == -1) {
        perror("Serwer: mq_open[1]");
        free(sciezka);
        return -1;
    }
    printf("Serwer:\n");
    for (;;) {
        wiad1 = (paczka1*)malloc(sizeof(paczka1));
        const size_t wielkosc1 = 64 * sizeof(paczka1);

        if ((liczba_bajtow = mq_receive(fd, (char*)wiad1, wielkosc1, NULL)) == -1) {
            perror("Serwer: mq_receive");
            free(wiad1);
            mq_close(fd);
            mq_unlink(sciezka);
            free(sciezka);
            return -1;
        }
        printf("Serwer: Przeczytalem %ld bajtow od klienta %d na glownej kolejce.\n", liczba_bajtow, (int)wiad1->pid);

        if ((pid = fork()) == -1) {
            perror("Serwer: Wywolanie funkcji fork nie powiodlo sie.\n");
            return -1;
        }

        if (pid == 0) {
            free(sciezka);

            mqd_t fd2;
            char sciezka_1[20];
            int cpid = (int)wiad1->pid;
            sprintf(sciezka_1, "/kolejka_1_%d", cpid);
            if ((fd2 = mq_open(sciezka_1, O_RDONLY, 0600)) == -1) {
                perror("Serwer: dziecko: mq_open()[2]");
                return -1;
            }

            paczka2* praca = (paczka2*)malloc(sizeof(paczka2));
            const size_t wielkosc2 = 64 * sizeof(paczka2);

            if ((liczba_bajtow = mq_receive(fd2, (char*)praca, wielkosc2, 0)) == -1) {
                perror("Serwer: mq_receive");
                mq_close(fd2);
                free(praca);
                return -1;
            }
            mq_close(fd2);

            mqd_t fd3;
            char sciezka_2[20];
            sprintf(sciezka_2, "/kolejka_2_%d", cpid);
            if ((fd3 = mq_open(sciezka_2, O_WRONLY, 0600)) == -1) {
                perror("Klient: mq_open[2]");
                free(praca);
                return -1;
            }

            printf("Serwer, proces potomny: Przeczytalem %ld bajtow od klienta.\n", liczba_bajtow);
            praca->melement *= praca->melement;
            praca->mtype = 3;

            const size_t wielkosc3 = sizeof(paczka2);

            if ((liczba_bajtow = mq_send(fd3, (char*)praca, wielkosc3, 0)) == -1) {
                perror("Serwer: dziecko: mq_send()[2]\n");
                free(praca);
                return -1;
            }

            mq_close(fd3);
            free(praca);
            return 0;
        }
    }
}


