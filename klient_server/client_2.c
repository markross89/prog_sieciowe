/*
Klient oparty na kolejkach komunikatow dla serwera wspolbieznego.
*/

#include<unistd.h>
#include<stdio.h>
#include<sys/uio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>
#include<string.h>

// deklaracja struktury dla pierwszej wiadomosci 
typedef struct mymesg1
{
    // struktura wiadomosci musi zaczynac sie od zmiennej typu long ktora okresla pryiorytet wiadomosci 
    long mtype;
    pid_t pid;
} paczka1;

// deklaracja struktury dla drugiej wiadomosci
typedef struct mymesg2
{
    // struktura wiadomosci musi zaczynac sie od zmiennej typu long ktora okresla pryiorytet wiadomosci 
    long mtype;
    double melement;
} paczka2;

int main(int argc, char* argv[])
{
    int f, f1;
    key_t klucz_kolejka;
    char* sciezka;
    int czy_czekac = 0, liczba_sekund;
    pid_t pid = getpid();
    const size_t wielkosc1 = sizeof(paczka1) - sizeof(long);

    if (argc > 1)
    {
        // sprawdzamy czy istnieje pierwszy argument czyli ile sekund program zostanie odlozony w czasie
        czy_czekac = 1;
        liczba_sekund = strtol(argv[1], NULL, 10);
    }
    if (argc > 2)
    {
        // sprawdzamy czy istnieje 2 argument czyli sciezka do pliku z kolejka wiadomosci i zapisuje ja w zmiennej
        sciezka = strndup(argv[2], strlen(argv[2]));
    }
    else
    {
        // podaje swoja sciezke jesli brak argumentu
        sciezka = strndup("./fifos/kolejka_serwer", strlen("./fifos/kolejka_serwer"));
    }

    // tworzy token taki sam jak w serwerze
    if ((klucz_kolejka = ftok(sciezka, 0)) == -1)
    {
        perror("Klient: Blad funkcji ftok.\n");
        // jesli cos jest nie tak usuwamy plik z kolejka i konczymy
        free(sciezka);
        return -1;
    }

    // otwieramy kolejke wiadomosci w pliku utworzonym na serwerze, 
    // w tym procesie mamy do niej dostep generujac taki sam token - funkcja ftok z tymi samymi parametrami
    if ((f = msgget(klucz_kolejka, 0600)) == -1)
    {
        perror("Klient: Blad funkcji msgget.\n");
        //free(sciezka);  ???
        return -1;
    }

    // zamawiamy pamiec dla wiadomosci
    paczka1* wiad = (paczka1*)malloc(sizeof(paczka1));
    ssize_t liczba_bajtow;

    // tworzymy nowy token do tego samego pliku
    if ((klucz_kolejka = ftok(sciezka, pid)) == -1)
    {
        perror("Klient: Blad funkcji ftok.\n");
        // jesli cos poszlo nie tak to zwalniamy pamiec wiadomosci
        free(wiad);
        return -1;
    }

    // usuwamy plik z kolejka wiadomosci ????????????????
    //free(sciezka);

    // otwieramy nowa kolejke wiadomosci z opcjami IPC_CREATE | IPC_EXCL dzieki ktorym jesli nie istnieje to zostanie stworzona
    if ((f1 = msgget(klucz_kolejka, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        perror("Klient: Blad funkcji msgget.\n");
        return -1;
    }

    // ustawiamy pryiorytet na 1
    wiad->mtype = 1;
    // zapisujemy numer procesu
    wiad->pid = pid;

    // wysylamy wiadomosc do pierwszej kolejki wiadomosci - f
    if (msgsnd(f, wiad, wielkosc1, 0) == -1)
    {
        perror("Klient: Blad funkcji msgsnd.\n");
        free(wiad);
        return -1;
    }

    // zwalniamy pamiec wyslanej wiadomosci - jest juz nie potrzebna
    free(wiad);
    // kolejki f nie usuwamy bo jest ciagle uzywana na serwerze

    paczka2* praca;
    const size_t wielkosc2 = sizeof(paczka2) - sizeof(long);
    double a = 0;

    printf("Klient: Podaj liczbe: ");
    scanf("%lf", &a);
    // zamawiamy pamiec dla drugiej wiadomosci 
    praca = (paczka2*)malloc(sizeof(paczka2));
    // ustawiamy pryiorytet na 2 i w kolejnej lini zapisujemy wartosc a
    praca->mtype = 1;
    praca->melement = a;

    // sprawdzamy czy istnieje 2 argument funkcji main czyli czas oczekiwania
    if (czy_czekac)
    {
        printf("Klient: Oczekuje %d sekund na wyslanie wiadomosci do serwera.\n", liczba_sekund);
        // jesli tak to usypiamy proces na podana liczbe sekund
        sleep(liczba_sekund);
    }
    // wysylamy wiadomosc do drugiej kolejki 
    if (msgsnd(f1, praca, wielkosc2, 0) == -1)
    {
        // jesli cos jest nie tak to zwalniamy pamiec wiadomosci
        free(praca);
        // jesli cos jest nie tak to usuwamy kolejke f1 ktora stworzylismy w tym procesie opcja IPC_RMID
        msgctl(f1, IPC_RMID, NULL);
        return -1;
    }
    
    // zapisujemy wiadomosc ktora otrzymalismy z serwera przez kolejke f1 i zapisujemy ja w strukturze praca
    // pryiorytet maja wiadomosci z numerem 3  
    if ((liczba_bajtow = msgrcv(f1, praca, wielkosc2, 0, 0)) == -1)
    {
        free(praca);
        msgctl(f1, IPC_RMID, NULL);
        return -1;
    }

    printf("Klient: Odebralem %ld bajtow od serwera.\n", liczba_bajtow);
    printf("Klient: Wynik: %lf^2 = %lf.\n", a, praca->melement);
    // na koniec procesu zwalniamy pamiec wiadomosci i linijke nizej usuwamy kolejke wiadomosci f1
    free(praca);
    msgctl(f1, IPC_RMID, NULL);
    return 0;
}