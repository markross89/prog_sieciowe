/*
Serwer datagramowy, internetowy, wspolbiezny.
*/

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<stdlib.h>
#include<signal.h>

typedef struct sockaddr SockAddr;
typedef struct sockaddr_in SockAddr_in;
int gniazdo;

/*
1 czas
2 dzien miesiac rok
3 dzien miesiac rok dzien_tygodnia
4 dzien miesiac rok dzien_tygodnia czas
5 dzien miesiac rok godzina
*/

char* zamien_date(time_t* a, int b) {
    char* wynik, * pomoc;

    pomoc = ctime(a);
    switch (b) {
    default:
        wynik = strndup(pomoc, strlen(pomoc));
        wynik[strlen(wynik) - 1] = '\0';
        break;
    case 1:
    {
        wynik = (char*)malloc(9 * sizeof(char));
        strncpy(wynik, &(pomoc[11]), 8);
        wynik[8] = '\0';
        break;
    }
    case 2:
    {
        wynik = (char*)malloc(12 * sizeof(char));
        strncpy(wynik, &(pomoc[8]), 2);
        wynik[2] = ' ';
        strncpy(&(wynik[3]), &(pomoc[4]), 3);
        wynik[6] = ' ';
        strncpy(&(wynik[7]), &(pomoc[20]), 4);
        wynik[11] = '\0';
        break;
    }
    case 3:
    {
        wynik = (char*)malloc(16 * sizeof(char));
        strncpy(wynik, &(pomoc[8]), 2);
        wynik[2] = ' ';
        strncpy(&(wynik[3]), &(pomoc[4]), 3);
        wynik[6] = ' ';
        strncpy(&(wynik[7]), &(pomoc[20]), 4);
        wynik[11] = ' ';
        strncpy(&(wynik[12]), pomoc, 3);
        wynik[15] = '\0';
        break;
    }
    case 4:
    {
        wynik = (char*)malloc(25 * sizeof(char));
        strncpy(wynik, &(pomoc[8]), 2);
        wynik[2] = ' ';
        strncpy(&(wynik[3]), &(pomoc[4]), 3);
        wynik[6] = ' ';
        strncpy(&(wynik[7]), &(pomoc[20]), 4);
        wynik[11] = ' ';
        strncpy(&(wynik[12]), pomoc, 3);
        wynik[15] = ' ';
        strncpy(&(wynik[16]), &(pomoc[11]), 8);
        wynik[24] = '\0';
        break;
    }
    case 5:
    {
        wynik = (char*)malloc(21 * sizeof(char));
        strncpy(wynik, &(pomoc[8]), 2);
        wynik[2] = ' ';
        strncpy(&(wynik[3]), &(pomoc[4]), 3);
        wynik[6] = ' ';
        strncpy(&(wynik[7]), &(pomoc[20]), 4);
        wynik[11] = ' ';
        strncpy(&(wynik[12]), &(pomoc[11]), 8);
        wynik[20] = '\0';
        break;
    }
    }
    return wynik;
}
// obsluga sygnalu
void al(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("Koniec pracy serwera.\n");
        close(gniazdo);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    int gniazdo1;
    socklen_t i;
    SockAddr_in adres, adres_kli, adres_serw;

    signal(SIGTERM, al);
    signal(SIGINT, al);
    sigignore(SIGCHLD);

    // otwieram gniazdo datagramowe internetowe i zapisuje deskryptor
    if ((gniazdo = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Serwer: Blad wywolania funkcji socket.\n");
        return -1;
    }

    // zeruje pamiec w strukturze adres
    bzero((char*)&adres, sizeof(SockAddr_in));
    // ustawia rodzine protokolow na ipv4
    adres.sin_family = AF_INET;
    if (argc > 1) {
        // ustawia port na podstawie argumentu
        adres.sin_port = htons(strtol(argv[1], NULL, 10));
    }
    else {
        // ustawia domyslny port jesli brak argumentu
        adres.sin_port = htons(0);
    }
    // ustawia IP adres na 0 czyli wszystkie adresy IP maszyny w tym 127.0.0.1 czyli lokalhost
    adres.sin_addr.s_addr = htonl(INADDR_ANY);
    i = sizeof(SockAddr_in);
    // przylacza gniazdo do wartosci podanych w adresie
    if (bind(gniazdo, (SockAddr*)&adres, i) < 0) {
        perror("Serwer: Blad dowiazania gniazda.\n");
        close(gniazdo);
        return -1;
    }

    // sprawdzenie czy adres zostal pomyslnie dodany i czy gniazdo jest pomyslnie otwarte ?????
    if (getsockname(gniazdo, (SockAddr*)&adres, &i) != 0) {
        perror("Serwer: Blad funkcji getsockname.\n");
        close(gniazdo);
        return -1;
    }

    char* t, * t1;
    int pomoc, pomoc1, plik;
    time_t czas;
    double argument;

    // drukujemy nr portu
    printf("Nr portu: %d\n", ntohs(adres.sin_port));
    // otwieramy plik w ktorym bedziemy zapisywac historie polaczen
    plik = open("./historia_polaczen.txt", O_WRONLY | O_CREAT, 0600);
    // zamawiamy miejsce w pamieci dynamicznej dla zmiennych 
    t = (char*)malloc(100 * sizeof(char));
    t1 = (char*)malloc(10 * sizeof(char));
    // zapisujemy text do pierwszej zmiennej
    sprintf(t, " Adres Port Pid Kiedy Opcja\n");
    // zapisujemy zmienna do pliku
    write(plik, t, strlen(t));
    // zmieniamy text pierwszej zmiennej
    sprintf(t, "------------------------------------------------------------\n");
    // zapisujemy raz jeszcze do pliku
    write(plik, t, strlen(t));
    for (;;) {
        // czytamy dane od klienta i zapisujemy je do drugiej zmiennej, a adres do struktury adresu
        if (recvfrom(gniazdo, t1, 10, 0, (SockAddr*)&adres_kli, &i) < 0) {
            perror("Serwer: Blad funkcji recfrom.\n");
            continue;
        }
        else {
            // rozdzielamy proces
            if (fork() == 0) {
                // zamykamy niepotzrebne juz gniazdo w procesie potomnym
                close(gniazdo);
                // zmieniamy otrzymana informacje od klienta na wartosc liczbowa
                pomoc = strtol(t1, NULL, 10);
                // przypisujemy zmiennej obecny czas
                czas = time(NULL);
                // zmieniamy  wartosc otrzymanej informacji w pozycji [6] na wartosc liczbowa
                pomoc1 = strtol(&(t1[6]), NULL, 10);
                // zapisujemy do pierwszej zmiennej adres, port, ...., czas, ....
                sprintf(t, "%15s %5d %5d %s %d\n", inet_ntoa(adres_kli.sin_addr), ntohs(adres_kli.sin_port),
                    pomoc, zamien_date(&czas, 5), pomoc1);
                // zapisujemy zmienna do pliku
                write(plik, t, strlen(t));
                // zapisuejmy dane do pierwszej zmiennej
                sprintf(t, "%s", zamien_date(&czas, pomoc1));
                // otwieramy kolejne gniazdo
                if ((gniazdo1 = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
                    perror("Serwer, proces potomny: Blad wywolania funkcji socket.\n");
                    return -1;
                }

                // zerujemy adres serwera jako dobra praktyka w celu unikniecia pomylek i bledow
                bzero((char*)&adres_serw, sizeof(SockAddr_in));
                // ustawiamy adres serwera
                adres_serw.sin_family = AF_INET;
                adres_serw.sin_addr.s_addr = htonl(INADDR_ANY);
                adres_serw.sin_port = htons(0);
                i = sizeof(SockAddr_in);
                // przylaczamy gniazdo wedlug danych z adresu
                if (bind(gniazdo1, (SockAddr*)&adres_serw, i) < 0) {
                    perror("Serwer, proces potomny: Blad dowiazania gniazda.\n");
                    close(gniazdo1);
                    return -1;
                }

                // sprawdzamy czy adres i gniazdo sa poprawne
                if (getsockname(gniazdo1, (SockAddr*)&adres_serw, &i) != 0) {
                    perror("Serwer, proces potomny: Blad funkcji getsockname.\n");
                    close(gniazdo1);
                    return -1;
                }

                // wysylamy adres portu serwera w procesie potomnym do klienta
                if (sendto(gniazdo, &(adres_serw.sin_port), sizeof(in_port_t), 0,
                    (SockAddr*)&adres_kli, i) < 0) {
                    perror("Serwer, proces potomny: Blad funkcji sendto #1.\n");
                    close(gniazdo1);
                    return -1;
                }

                // drukujemy info z czasem wysylki
                printf("Serwer: wysylam %s.\n", t);
                // ten sam czas wysylki wysylamy do klienta
                if (sendto(gniazdo1, t, 25, 0, (SockAddr*)&adres_kli, i) < 0) {
                    perror("Serwer, proces potomny: Blad funkcji sendto #2.\n");
                    close(gniazdo1);
                    return -1;
                }
                // odbieramy dane od klienta i zapisujemy do zmiennej 
                if (recvfrom(gniazdo1, &argument, sizeof(double), 0,
                    (SockAddr*)&adres_kli, &i) < 0) {
                    perror("Klient: Blad funkcji recfrom.\n");
                    close(gniazdo1);
                    return -1;
                }
                // drukujemy dane otrzymane od klienta
                printf("Otrzymalem liczbe: %lf.\n", argument);
                // podnosimy liczbe typu double otrzymana od klienta do kwadratu
                argument *= argument;
                // wysylamy do klienta zmienione dane
                if (sendto(gniazdo1, &argument, sizeof(double), 0, (SockAddr*)&adres_kli, i) < 0) {
                    perror("Klient: Blad funkcji sendto.\n");
                    close(gniazdo1);
                    return -1;
                }
                // zamykamy gniazdo
                close(gniazdo1);
                return 0;
            }
            else {
                bzero((char*)&adres_kli, sizeof(SockAddr_in));
                sigignore(SIGCHLD);
            }
        }
    }
}


