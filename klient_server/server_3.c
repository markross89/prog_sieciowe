/*
Serwer sekwencyjny oparty na semaforach i pamieci wspoldzielonej (wersja standardowa). Do synchronizacji dostepu do pamieci uzywane sa dwa semafory: 0 (wartosc 0, gdy klient nie ma dostepu do wspolnej pamieci i warotsc 1, gdy klient ma dostep do wspolnej pamieci), 1 (wartosc 0, gdy serwer nie ma dostepu do wspolnej pamieci i wartosc 1, gdy serwer ma dostep do wspolnej pamieci).
*/

#include<stdio.h>
#include<string.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>

// struktora ktora wykonuje operacje na semaforze

// struct sembuf
// {
//         // numer semaforu na ktorym chcemy operowac
//     short sem_num;
//         // operacja jaka chcemy wykonac: -1 - oznacza dekrementacje (wait()),  +1 - oznacza inkrementacja (post())
//     short sem_op;
//         // mozna przypisac flagi: 0 - czekaj na semafor jesli niedostepny, IPC_NOWAIT - nie czekaj na semafor i nie cofaj dotychczasowych zmian, 
//         // SEM_UNDO - cofnij zmiany bez wzgledu na konsekwencje
//     short sem_flg;
// }

typedef struct sembuf bufor_semaforow;
int id_semafor;
int id_pamieci;

// zmienna globalna do bloku pamieci podlaczanego przez shmat z pomoca jadra systemu w wybranym przez siebie (nieznanym) miejscu. Blok jest zwalniany przy pomocy chmdt
char* pamiec_wspolna;

// zmienna globalna o zmiennej wielkosci  - alokowana na stercie (heap - pamiec dynamiczna) - pamiec musi zostac zwolniona prze funkcje free()
char* sciezka;

int czy_skasowac = 1;

// obsluga sygnalow wymuszajacych zamkniecie procesu
void al(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("Serwer: Koniec pracy serwera.\n");
        // odlacza blok pamieci wspoldzielonej
        shmdt(pamiec_wspolna);
        // usuwa blok pamieci wspoldzielonej
        shmctl(id_pamieci, IPC_RMID, NULL);
        // usuwa zbior semaforow - wskazanie 0 czyli pierwszy semafor jest tu ignorowane
        semctl(id_semafor, 0, IPC_RMID, NULL);
        if (czy_skasowac) {
            // usuwa plik ktory sluzy tylko do odwolania sie do miejsca w pamieci gdzie umieszczony jest blok pamieci wspoldzielonej
            unlink(sciezka);
        }
        // zwalnia dynamicznie alokowana pamiec zmiennej globalnej o zmiennej wielkosci w ktorej zapisana jest sciezka do pliku
        free(sciezka);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    key_t klucz_semafory, klucz_pamiec_wspolna;
    bufor_semaforow ustaw_semafory;
    int plik = 0;

    // obsluga sygnalow
    signal(SIGTERM, al);
    signal(SIGINT, al);

    if (argc > 1) {
        // przypisanie argumentu do sciezki
        sciezka = strndup(argv[1], strlen(argv[1]));
    }
    else {
        // jesli brak argumentu zapisanie wlasnej sciezki
        sciezka = strndup("./fifos/sem_serwer", strlen("./fifos/sem_serwer"));
    }
    // sprawdzamy czy plik ze sciezki istnieje aby ustawic 
    if (access(sciezka, F_OK) == 0) {
        czy_skasowac = 0;
    }
    // jesli nie istnieje to tworzymy nowy plik
    if (czy_skasowac && ((plik = open(sciezka, O_CREAT | O_EXCL, 0600)) == -1)) {
        fprintf(stderr, "Serwer: Blad utworzenia pliku %s: %s.\n", sciezka, strerror(errno));
        // przy kazdym error zwalniamy dynamicznie alokowana pamiec sciezki
        free(sciezka);
        return -1;
    }

    // zamykamy plik ktorego bedziemy uzywali tylko jako wskaznik do pamieci wspoldzdielonej
    close(plik);

    // tworzymy dwa tokeny do pamieci wspoldzielonej
    if ((klucz_semafory = ftok(sciezka, 0)) == -1 || (klucz_pamiec_wspolna = ftok(sciezka, 1)) == -1) {
        perror("Serwer: Blad funkcji ftok.\n");
        if (czy_skasowac) {
            // usuwamy plik jesli cos idzie nie tak
            unlink(sciezka);
        }
        free(sciezka);
        return -1;
    }

    // tworzy lub otwiera istniejaca pamiec wspoldzielona oraz zwraca jej identyfikator
    if ((id_pamieci = shmget(klucz_pamiec_wspolna, 50 * sizeof(char), 0600 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Serwer: Blad funkcji shmget.\n");
        if (czy_skasowac) {
            unlink(sciezka);
        }
        free(sciezka);
        return -1;
    }

    // podlaczamy pamiec wspoldzielona 
    pamiec_wspolna = shmat(id_pamieci, (char*)0, 0);

    // tworzy lub otwiera istniejacy zbior semaforow oraz zwraca identyfikator tego zbioru - w tym przypadku 2 semafory
    if ((id_semafor = semget(klucz_semafory, 2, 0600 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Serwer: Blad funkcji semget.\n");
        // w razie bledu odlaczamy pamiec wspoldzielona
        shmdt(pamiec_wspolna);
        // oraz usuwamy pamiec wspoldzielona
        shmctl(id_pamieci, IPC_RMID, NULL);
        if (czy_skasowac) {
            // usuwamy plik wskazany przez sciezke
            unlink(sciezka);
        }
        // zwalniamy dynamicznie alokowana pamiec sciezki
        free(sciezka);
        return -1;
    }

    unsigned short wartosci_poczatkowe_semaforow[] = { 1, 0 };
    //  semctl(identyfikator semaforow, numer semafora, SETALL - ustawia semafory do podanych wartosci (ignoruje nr semafora), zbior wartosci)
    // funkcja ustawia semafor lub semafory do zadanych wartosci
    // opcje manipulacji to:  +1 (post) - ustawienie semafora na 1        -1 (wait) - ustawienie semafora na 0 
    // jesli wywolamy -1 na semaforze o wartosci 0 to blokujemy proces do momentu az gdzies zmienimy jego wartosc na +1
    if ((semctl(id_semafor, 0, SETALL, wartosci_poczatkowe_semaforow)) == -1) {
        perror("Serwer: Blad funkcji semctl.\n");
        // usuwa zbior semaforow - wskazanie 0 czyli pierwszy semafor jest tu ignorowane
        semctl(id_semafor, 0, IPC_RMID, NULL);
        // odlacza pamiec wspoldzielona
        shmdt(pamiec_wspolna);
        // usuwa pamiec wspoldzielona
        shmctl(id_pamieci, IPC_RMID, NULL);
        if (czy_skasowac) {
            unlink(sciezka);
        }
        free(sciezka);
        return -1;
    }

    for (;;) {
        // ustawiamy strukture sluzaca do modyfikacji semaforu klienta
        ustaw_semafory.sem_num = 1;
        // wywolujemy -1 na semaforze ktory ma opcje zero czyli chcemy wywolac blokowanie procesu
        ustaw_semafory.sem_op = -1;
        ustaw_semafory.sem_flg = 0;
        printf("Serwer: Oczekuje na wiadomosc od klienta.\n");
        // semop(identyfikator zbioru semaforow,   lista struktur sem_buf (w tym przypadku 1),  ilosc struktur (w tym przypadku 1))
        // w tym miejscu odpalamy semafor ktory czeka na odblokowanie - proces zawieszony
        if (semop(id_semafor, &ustaw_semafory, 1) == -1) {
            perror("Server: Blad oczekiwania na otwarcie semafora do odczytu dla serwera (oczekiwanie serwera), funkcja semop.\n");
            // w przypadku bledu odlacza pamiec wspoldzielona
            shmdt(pamiec_wspolna);
            // usuwa blok pamieci wspoldzielonej
            shmctl(id_pamieci, IPC_RMID, NULL);
            // usuwa zbior semaforow - wskazanie 0 czyli pierwszy semafor jest tu ignorowane
            semctl(id_semafor, 0, IPC_RMID, NULL);
            if (czy_skasowac) {
                unlink(sciezka);
            }
            free(sciezka);
            return -1;
        }

        // klient zmienia wartosc semafora serwera na 0 oraz wartosc swojego semafora na 1
        // nastepnie wartosc semafora klienta zostaje ustawiona na 0 przez proces serwera ktory czekal 
        // oba semafory sa ustawione na 0 wiec pamiec jest dostepna i pobieramy dane
        char* wiadomosc = strdup(pamiec_wspolna);
        printf("Serwer: Wiadomosc od klienta: %s.\n", wiadomosc);

        // ustawiamy opcje struktury semafora serweru aby wrocic do ustawien poczatkowych czyli 1, 0
        ustaw_semafory.sem_num = 0;
        // +1 czyli post() 
        ustaw_semafory.sem_op = 1;
        // blokujemy
        ustaw_semafory.sem_flg = 0;

        // ustawia semafor lub semafory do zadanych parametrow okreslonych w strukturze
        if (semop(id_semafor, &ustaw_semafory, 1) == -1) {
            // mimo iz zmienna zostala zadeklarowana jako zmienna lokalna to wskazuje na pamiec dynamiczna wiec rowniez trzeba pamietac aby ja zwolnic
            free(wiadomosc);
            perror("Serwer: Blad oczekiwania na otwarcie semafora do zapisu dla klienta (zwolnienie dla klienta), funkcja semop.\n");
            // odlaczamy pamiec
            shmdt(pamiec_wspolna);
            // usuwamy pamiec
            shmctl(id_pamieci, IPC_RMID, NULL);
            // usuwa zbior semaforow - wskazanie 0 czyli pierwszy semafor jest tu ignorowane
            semctl(id_semafor, 0, IPC_RMID, NULL);
            if (czy_skasowac) {
                unlink(sciezka);
            }
            free(sciezka);
            return -1;
        }
        
        // zwalniamy pamiec dynamiczna wskazana przez wiadomosc
        free(wiadomosc);
    }
}
