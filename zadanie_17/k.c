/*
Klient oparty na kolejkach komunikatow dla serwera wspolbieznego.
*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mqueue.h>

typedef struct mymesg1
{
    long mtype;
    pid_t pid;
} paczka1;
typedef struct mymesg2
{
    long mtype;
    double melement;
} paczka2;

int main(int argc, char* argv[])
{
    int f, f1, f2;
    char* sciezka;
    int czy_czekac = 0, liczba_sekund;
    pid_t pid = getpid();
    ssize_t liczba_bajtow;

    if (argc > 1)
    {
        czy_czekac = 1;
        liczba_sekund = strtol(argv[1], NULL, 10);
    }
    if (argc > 2)
    {
        sciezka = strndup(argv[2], strlen(argv[2]));
    }
    else
    {
        sciezka = strndup("/kolejka", strlen("/kolejka"));
    }

    if ((f = mq_open(sciezka, O_WRONLY, 0600)) == -1) {
        perror("Klient: mq_open[f]");
        return -1;
    }

    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 5, .mq_msgsize = 100 };

    char sciezka_1[20];
    sprintf(sciezka_1, "/kolejka_1_%d", (int)getpid());
    mq_unlink(sciezka_1);
    if ((f1 = mq_open(sciezka_1, O_CREAT | O_WRONLY, 0600, &attr)) == -1) {
        perror("Klient: mq_open[f1]");
        mq_close(f);
        return -1;
    }

    char sciezka_2[20];
    sprintf(sciezka_2, "/kolejka_2_%d", (int)getpid());
    if ((f2 = mq_open(sciezka_2, O_CREAT | O_RDONLY, 0600, &attr)) == -1) {
        perror("Klient: mq_open[f2]");
        mq_close(f);
        mq_close(f1);
        mq_unlink(sciezka_1);
        return -1;
    }

    paczka1* wiad = (paczka1*)malloc(sizeof(paczka1));
    const size_t wielkosc1 = sizeof(paczka1);

    wiad->mtype = 1;
    wiad->pid = pid;

    if ((liczba_bajtow = mq_send(f, (char*)wiad, wielkosc1, 0)) == -1) {
        perror("Klient: mq_send()[f]\n");
        mq_close(f);
        mq_close(f1);
        mq_close(f2);
        mq_unlink(sciezka_1);
        mq_unlink(sciezka_2);
        free(wiad);
        return -1;
    }
    free(wiad);
    mq_close(f);

    paczka2* praca = (paczka2*)malloc(sizeof(paczka2));
    const size_t wielkosc2 = sizeof(paczka2);
    double a = 0;

    printf("Klient: Podaj liczbe: ");
    scanf("%lf", &a);
    praca->mtype = 0;
    praca->melement = a;
    if (czy_czekac)
    {
        printf("Klient: Oczekuje %d sekund na wyslanie wiadomosci do serwera.\n", liczba_sekund);
        sleep(liczba_sekund);
    }
    if ((liczba_bajtow = mq_send(f1, (char*)praca, wielkosc2, 0)) == -1) {
        perror("Klient: mq_send()[f1]\n");
        mq_close(f1);
        mq_close(f2);
        mq_unlink(sciezka_2);
        mq_unlink(sciezka_1);
        free(praca);
        return -1;
    }
    mq_close(f1);

    const size_t wielkosc3 = 64 * sizeof(paczka2);

    if ((liczba_bajtow = mq_receive(f2, (char*)praca, wielkosc3, 0)) == -1) {
        perror("Klient: mq_receive");
        mq_close(f2);
        mq_unlink(sciezka_2);
        mq_unlink(sciezka_1);
        free(praca);
        return -1;
    }
    mq_close(f2);

    printf("Klient: Odebralem %ld bajtow od serwera.\n", liczba_bajtow);
    printf("Klient: Wynik: %lf^2 = %lf.\n", a, praca->melement);

    mq_unlink(sciezka_2);
    mq_unlink(sciezka_1);
    free(praca);

    return 0;
}
