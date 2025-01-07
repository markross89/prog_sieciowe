/*
Serwer wspolbiezny oparty na kolejekach komunikatow.
*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/uio.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

typedef struct mymesg1 {
    long mtype;
    pid_t pid;
} paczka1;

typedef struct mymesg2 {
    long mtype;
    double melement;
} paczka2;

int f;
paczka1* wiad;
paczka2* praca;
char* sciezka;
int czy_skasowac = 1;

void wyczysc()
{
    if (czy_skasowac) {
        unlink(sciezka);
    }
    free(sciezka);
    free(wiad);
    free(praca);
    msgctl(f, IPC_RMID, NULL);
    printf("Pamiec wyczyszczona\n");
}

void zakoncz(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("\nSerwer: Koniec pracy serwera.\n");
        wyczysc();
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    pid_t pid;
    key_t klucz_kolejka;
    ssize_t liczba_bajtow;
    int plik;

    signal(SIGTERM, zakoncz);
    signal(SIGINT, zakoncz);
    sigignore(SIGCHLD);

    if (argc > 1) {
        sciezka = strndup(argv[1], strlen(argv[1]));
    }
    else {
        sciezka = strndup("./kolejka_serwer", strlen("./kolejka_serwer"));
    }
    if (access(sciezka, F_OK) == 0) {
        czy_skasowac = 0;
    }

    if (czy_skasowac && ((plik = open(sciezka, O_CREAT | O_EXCL, 0600)) == -1)) {
        fprintf(stderr, "Serwer: Blad utworzenia pliku %s: %s.\n", sciezka, strerror(errno));
        free(sciezka);
        return -1;
    }
    close(plik);

    if ((klucz_kolejka = ftok(sciezka, 0)) == -1) {
        perror("Serwer: Blad funkcji ftok.\n");
        if (czy_skasowac) {
            unlink(sciezka);
        }
        free(sciezka);
        return -1;
    }

    if ((f = msgget(klucz_kolejka, 0600 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Serwer: Blad funkcji msgget.\n");
        if (czy_skasowac) {
            unlink(sciezka);
        }
        free(sciezka);
        return -1;
    }

    wiad = (paczka1*)malloc(sizeof(paczka1));
    const size_t wielkosc1 = sizeof(paczka1) - sizeof(long);  // dlaczego - siezeof(long)?

    praca = (paczka2*)malloc(sizeof(paczka2));
    const size_t wielkosc2 = sizeof(paczka2) - sizeof(long);

    for (;;) {

        if ((liczba_bajtow = msgrcv(f, wiad, wielkosc1, 1, 0)) == -1) {
            perror("Serwer: Blad funkcji msgrcv[1].\n");
            wyczysc();
            return -1;
        }

        printf("Serwer: Przeczytalem %ld bajtow od klienta nr %d, na glownej kolejce.\n", liczba_bajtow, (int)wiad->pid);

        if ((liczba_bajtow = msgrcv(f, praca, wielkosc2, 2, 0)) == -1) {
            perror("Serwer: Blad funkcji msgrcv[2].\n");
            wyczysc();
            return -1;
        }

        printf("Serwer: Otrzymana wartosc: %lf\n", praca->melement);

        praca->melement *= praca->melement;
        praca->mtype = 3;

        if (msgsnd(f, praca, wielkosc2, 0) == -1) {
            perror("Serwer: Blad funkcji msgsnd.\n");
            wyczysc();
            return -1;
        }
    }
}

