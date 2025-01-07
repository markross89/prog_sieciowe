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

// struktura zawierajaca wiadomosc - "paczka1" jest nazwa ktora bedziemy uzywac zamiast "struct mymesg1"
typedef struct mymesg1 {
    // konieczna dla tej struktury opcja priorytetu
    long mtype;
    pid_t pid;
} paczka1;

// inna struktura zawierajaca wiadomosc - "paczka2" jest nazwa ktora bedziemy uzywac zamiast "struct mymesg2"
typedef struct mymesg2 {
    // konieczna dla tej struktury opcja priorytetu
    long mtype;
    double melement;
} paczka2;

int f;
paczka1* wiad;
char* sciezka;
int czy_skasowac = 1;

// funkcja obslugujaca sygnaly wymuszonego zamkniecia programu
void ala(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("Serwer: Koniec pracy serwera.\n");
        // zwalniamy pamiec struktury wiad
        free(wiad);
        // opcja IPC_RMID w funkcji msgctl usuwa kolejke komunikatow
        msgctl(f, IPC_RMID, NULL);
        if (czy_skasowac) {
            // usuwa plik zawierajacy kolejke komunikatow jesli go stworzylismy
            unlink(sciezka);
        }
        // usuwa plik zawirajacy kolejke komunikatow jesli byl dostepny juz wczesniej
        free(sciezka);
        // konczymy dzialanie programu i zwracamy 0 - brak bledu
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    pid_t pid;
    key_t klucz_kolejka;
    const size_t wielkosc1 = sizeof(paczka1) - sizeof(long);
    ssize_t liczba_bajtow;
    int plik = 0;

    // obsluga sygnalow zakonczenia programu
    signal(SIGTERM, ala);
    signal(SIGINT, ala);

    // ignorowanie sygnalu zakonczenia processu potomnego
    sigignore(SIGCHLD);

    if (argc > 1) {
        // jesli zostal podany argument w postaci sciezki do zapisu - zapisujemy go do zmiennej sciezka
        sciezka = strndup(argv[1], strlen(argv[1]));
    }
    else {
        // w przeciwnym wypadku program podaje swoja sciezke
        sciezka = strndup("./fifos/kolejka_serwer", strlen("./fifos/kolejka_serwer"));
    }
    // sprawdzamy czy w podanej sciezce istnieje dzialajacy plik. jesli tak funkcja zwraca 0 a wartosc zmiennej czy skasowac ustawiamy na 0 - false
    if (access(sciezka, F_OK) == 0) {
        czy_skasowac = 0;
    }

    // jesli plik istnieje to nie tworzymy nowego pliku
    // w innym przypadku otwieramy strumien do pliku o zadanej sciezce.  o_create tworzy plik a o_excl rzuca error jesli ten juz istnieje.
    if (czy_skasowac && ((plik = open(sciezka, O_CREAT | O_EXCL, 0600)) == -1)) {
        fprintf(stderr, "Serwer: Blad utworzenia pliku %s: %s.\n", sciezka, strerror(errno));
        // usuwamy plik jesli powstal z bledem ?
        free(sciezka);
        return -1;
    }
    // zamykamy strumien ktory otworzylismy tylko po to aby stworzyc plik
    // jesli uzylibysmy tylko open bez przypisania do pliku to mieli bysmy brak mozliwosci zamkniecia strumienia
    close(plik);

    // ftok tworzy strumien i generuje specjalny systemowy token potrzebny aby dostac sie do pliku pod zadana sciezka
    // zakres dzialania tego tokena to caly system a nie tylko ten program
    // jest on niezbedny dla obslugi semget, shmget, msgget
    if ((klucz_kolejka = ftok(sciezka, 0)) == -1) {
        perror("Serwer: Blad funkcji ftok.\n");
        if (czy_skasowac) {
            // usuwamy plik jesli utworzylismy nowy i cos poszlo nie tak ?
            unlink(sciezka);
        }
        // usuwamy plik jesli juz istnial i cos poszlo nie tak ?
        free(sciezka);
        return -1;
    }

    // msgget otwiera/tworzy kolejke wiadomosci o zadanych parametrach nawiazujaca do pliku utworzonego wczesniej i zwraca identyfikator kolejki
    // podobnie jak  funkcja open() nawiazujaca do kolejki fifo
    if ((f = msgget(klucz_kolejka, 0600 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Serwer: Blad funkcji msgget.\n");
        if (czy_skasowac) {
            // usuwamy plik jesli utworzylismy nowy i cos poszlo nie tak ?
            unlink(sciezka);
        }
        // usuwamy plik jesli juz istnial i cos poszlo nie tak ?
        free(sciezka);
        return -1;
    }
    // niekonczaca sie petla sprawia ze server dziala caly czas 
    for (;;) {

        // zamawiamy miejsce w pamieci dla wiadomosci 
        wiad = (paczka1*)malloc(sizeof(paczka1));

        // odczytujemy wiadomosc z kolejki f i zapisujemy do struct wiadomosc oraz zapisujemy liczbe zapisanyc bajtow
        // 5 argument oznacza ze jesli kolejka jest pusta proces bedzie blokowany do momentu pojawienia sie wiadomosci. jesli wpiszemy 1 to nie blokuje i przerywa proces
        // 4 argument - 1 oznacza pryiorytet wiadomosci zapisany w strukturze wiadomosci - jesli podamy 0 brak pryiorytetu 
        if ((liczba_bajtow = msgrcv(f, wiad, wielkosc1, 0, 0)) == -1) {
            perror("Serwer: Blad funkcji msgrcv.\n");
            if (czy_skasowac) {
                // usuwamy plik jesli utworzylismy nowy i cos poszlo nie tak ?
                unlink(sciezka);
            }
            // usuwamy plik jesli juz istnial i cos poszlo nie tak ?
            free(sciezka);
            // zwalniamy pamiec zamowiona dla wiad
            free(wiad);
            return -1;
        }

        printf("Serwer: Przeczytalem %ld bajtow od klienta na glownej kolejce.\n", liczba_bajtow);

        // tworzymy proces potmony
        if ((pid = fork()) == -1) {
            perror("Serwer: Wywolanie funkcji fork nie powiodlo sie.\n");
            return -1;
        }

        if (pid == 0) {
            int f1;

            //free(sciezka); -------------------- BLAD usuwamy plik aby zachwile z niego skorzystac

            // generujemy nowy token tym razem zamiast 0 mamy nr procesu klienta - klient tworzy ta kolejke a my z niej korzystamy
            if ((klucz_kolejka = ftok(sciezka, wiad->pid)) == -1) {
                perror("Serwer, proces potomny: Blad funkcji ftok.\n");
                return -1;
            }

            // otwieramy kolejke 
            if ((f1 = msgget(klucz_kolejka, 0600)) == -1) {
                perror("Serwer, proces potomny : Nie moge utworzyc kolejki komunikatow dla klienta.\n");
                return -1;
            }

            // zamawiamy pamiec dla wiadomosci zwrotnej (struct praca)
            paczka2* praca = (paczka2*)malloc(sizeof(paczka2));
            // ustawiamy wielkosc wiadomosci
            const size_t wielkosc2 = sizeof(paczka2) - sizeof(long);


            // pobieramy dane z f1 i zapisujemy wiadomosc (struct praca)
            if ((liczba_bajtow = msgrcv(f1, praca, wielkosc2, 0, 0)) == -1) {
                perror("Serwer, proces potomny: Blad funkcji msgrcv.\n");
                return -1;
            }

            printf("Serwer, proces potomny: Przeczytalem %ld bajtow od klienta.\n", liczba_bajtow);

            // zmieniamy wiadomosc (struct praca)
            praca->melement *= praca->melement;
            // ustawiamy pryiorytet na 3
            praca->mtype = 1;

            // wysylamy zmieniona wiadomosc do kolejki f1
            if ((liczba_bajtow = msgsnd(f1, praca, wielkosc2, 0)) == -1) {
                perror("Serwer, proces potomny: Blad funkcji msgsnd.\n");
                return -1;
            }
            // zwalniamy pamiec przydzielona dla wiadomosci (struct praca)
            free(praca);
            return 0;

        }
        sigignore(SIGCHLD);  // czemu ignorujemy sygnal smierci potomka na poczatku i na koncu.
    }

    // nie konczymy programu returnem bo nie ma sensu. Serwer ma dzialac w nieskonczonosc a jesli chcemy wymusic zamkniecie robimy to przez SIGTERM albo SIGINT
    // a nastepnie obslugujemy te sygnaly.
}