/*
Klient oparty na semaforach i pamieci wspoldzielonej dla serwera sekwencyjnego (wersja standardowa). Do synchronizacji dostepu do pamieci uzywane sa dwa semafory: 0 (wartosc 0, gdy klient nie ma dostepu do wspolnej pamieci i warotsc 1, gdy klient ma dostep do wspolnej pamieci), 1 (wartosc 0, gdy serwer nie ma dostepu do wspolnej pamieci i wartosc 1, gdy serwer ma dostep do wspolnej pamieci).
*/

#include<sys/sem.h>
#include<sys/shm.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct sembuf bufor_semaforow;

int main(int argc, char* argv[]) {
    key_t klucz_semafory, klucz_pamiec_wspolna;
    bufor_semaforow ustaw_semafory;
    int id_semafor;
    int id_pamieci;
    char* sciezka;
    int czy_czekac = 0, liczba_sekund;
    pid_t pid = getpid();

    if (argc > 1) {
        czy_czekac = 1;
        liczba_sekund = strtol(argv[1], NULL, 10);
    }

    if (argc > 2) {
        sciezka = strndup(argv[2], strlen(argv[2]));
    }
    else {
        sciezka = strndup("./fifos/sem_serwer", strlen("./fifos/sem_serwer"));
    }

    if ((klucz_semafory = ftok(sciezka, 0)) == -1 || (klucz_pamiec_wspolna = ftok(sciezka, 1)) == -1) {
        perror("Klient: Blad funkcji ftok.\n");
        free(sciezka);
        return -1;
    }

    free(sciezka);
    // laczy sie ze zbiorem semaforow na serwerze przy pomocy klucza blizniaczego do tego w serwerze
    if ((id_semafor = semget(klucz_semafory, 2, 0600)) == -1) {
        perror("Klient: Blad funkcji semget.\n");
        return -1;
    }
    // semafor serwera poczatkowo mial wartos 1 - post czyli jest gotowy do uzycia dekrementujac jego wartos do zera odrazu mozemy wyslac dane 
    // w ten sposob mozemy wymusic blokowanie jesli server czeka na te dane i chce miec do nich dostep po pelnym zapisaniu moze wywolac wait na tym semaforze
    ustaw_semafory.sem_num = 0;
    ustaw_semafory.sem_op = -1;
    ustaw_semafory.sem_flg = 0;

    printf("Klient: Moj numer procesu to %d. Oczekuje na mozliwosc zapisu.\n", pid);
    // ustawiamy semafor podajac mu structure jako parametr i blokujemy pamiec do momentu zapisania danych
    if (semop(id_semafor, &ustaw_semafory, 1) == -1) {
        perror("Klient: Blad oczekiwania na otwarcie semafora do zapisu dla klienta (oczekiwanie klienta), funkcja semop.\n");
        return -1;
    }

    // laczymy sie z pamiecia wspoldzielona przy pomocy klucza blizniaczego do tego z serwera
    if ((id_pamieci = shmget(klucz_pamiec_wspolna, 50 * sizeof(char), 0600)) == -1) {
        perror("Klient: Blad funkcji shmget.\n");
        return -1;
    }

    // alokujemy pamiec dynamiczna dla wiadomosci ktora przekazemy na serwer
    char* wiadomosc = (char*)malloc(100 * sizeof(char));
    // podlaczamy pamiec wspoldzielona do wskaznika 
    char* pamiec_wspolna = shmat(id_pamieci, (char*)0, 0);

    // umieszczamy wiadomosc w zmiennej
    sprintf(wiadomosc, "Moj numer procesu to %d", pid);
    // kopiujemy wiadomosc do pamieci wspoldzielonej
    strcpy(pamiec_wspolna, wiadomosc);

    printf("Klient: Wiadomosc zapisana.\n");
    // zwalniamy pamiec wiadomosci
    free(wiadomosc);
    // odlaczamy pamiec wspoldzielona
    shmdt(pamiec_wspolna);

    if (czy_czekac) {
        printf("Klient: Oczekuje %d sekund na zwolnienie semafora do czytania dla serwera.\n", liczba_sekund);
        // jesli argument czas zostal podany to usypiamy proces o podana wartosc
        sleep(liczba_sekund);
    }

    // zmieniamy wartosc semafora klienta z 0 na 1 czyli udostepniamy pamiec serwerowi
    ustaw_semafory.sem_num = 1;
    ustaw_semafory.sem_op = 1;
    ustaw_semafory.sem_flg = 0;

    // odpalamy semafor 
    if (semop(id_semafor, &ustaw_semafory, 1) < 0) {
        perror("Klient: Blad oczekiwania na otwarcie semafora do odczytu dla serwera (zwolnienie dla serwera).\n");
        return -1;
    }
    return 0;
}