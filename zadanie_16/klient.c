#include<unistd.h>
#include<stdio.h>
#include<sys/uio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>
#include<string.h>


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
    int f;
    key_t klucz_kolejka;
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
        sciezka = strndup("./kolejka_serwer", strlen("./kolejka_serwer"));
    }

    if ((klucz_kolejka = ftok(sciezka, 0)) == -1)
    {
        perror("Klient: Blad funkcji ftok.\n");
        return -1;
    }

    if ((f = msgget(klucz_kolejka, 0666)) == -1)
    {
        perror("Klient: Blad funkcji msgget.\n");
        return -1;
    }

    paczka1* wiad = (paczka1*)malloc(sizeof(paczka1));
    const size_t wielkosc1 = sizeof(paczka1) - sizeof(long);

    wiad->mtype = 1;
    wiad->pid = pid;

    if (msgsnd(f, wiad, wielkosc1, 0) == -1)
    {
        perror("Klient: Blad funkcji msgsnd.\n");
        free(wiad);
        return -1;
    }
    free(wiad);


    double a;
    printf("Klient: Podaj liczbe: ");
    scanf("%lf", &a);

    paczka2* praca = (paczka2*)malloc(sizeof(paczka2));
    const size_t wielkosc2 = sizeof(paczka2) - sizeof(long);
 
    praca->mtype = 2;
    praca->melement = a;

    if (czy_czekac)
    {
        printf("Klient: Oczekuje %d sekund na wyslanie wiadomosci do serwera.\n", liczba_sekund);
        sleep(liczba_sekund);
    }
    if (msgsnd(f, praca, wielkosc2, 0) == -1)
    {
        free(praca);
        return -1;
    }

    if ((liczba_bajtow = msgrcv(f, praca, wielkosc2, 3, 0)) == -1)
    {
        free(praca);
        return -1;
    }

    printf("Klient: Odebralem %ld bajtow od serwera.\n", liczba_bajtow);
    printf("Klient: Wynik: %lf^2 = %lf.\n", a, praca->melement);
    
    free(praca);

    return 0;
}