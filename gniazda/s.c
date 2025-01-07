/*
Serwer wspolbiezny strumieniowy, internetowy.
*/

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>

// deklaracja struktury adresu
typedef struct sockaddr SockAddr;
// deklaracja struktury adresu internetowego
typedef struct sockaddr_in SockAddr_in;
int gniazdo;

// obsluga sygnalow
void al(int i) {
    if (i == SIGTERM || i == SIGINT) {
        printf("Serwer: Koniec pracy serwera.\n");
        close(gniazdo);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    int gniazdo1;
    socklen_t i;
    SockAddr_in adres, adres_k;
    // obsluga sygnalow
    signal(SIGTERM, al);
    signal(SIGINT, al);
    sigignore(SIGCHLD);
    // otwieramy ganiazdo
    if ((gniazdo = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Serwer: Blad wywolania funkcji socket.\n");
        return -1;
    }
    else {
        i = sizeof(SockAddr_in);
        // ustawiamy rodzine protokolow na ipv4
        adres.sin_family = AF_INET;
        if (argc > 1) {
            // ustawiamy port na podstawie parametru
            // htons tlumaczy wartosc liczbowa portu na wartosci zrozumiale dla struktury ares
            adres.sin_port = htons(strtol(argv[1], NULL, 10));
        }
        else {
            // jesli brak parametru ustawiamy domyslny port
            adres.sin_port = htons(0);
        }
        // mapujemy adres na zadane parametry czyli rodzine protokolow, nr portu, adres IP
        // INADDR_ANY ustawia adres IP na 0 co oznacza ze wszystkie adresy IP tej maszyny beda zmapowane na podany port (serwer/klient na jednym komputerze)
        // htonl przetwarza adres IP na dane zrozumiale dla struktury adres
        adres.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(gniazdo, (SockAddr*)&adres, i) == -1) {
            perror("Serwer: Blad dowiazania gniazda.\n");
            close(gniazdo);
            return -1;
        }

        // // funkcja zwraca adres podanego gniazda ale nie wiem po co tu jest
        // // zeby sprawdzic adres zostal dobrze ustawiony?
        // if (getsockname(gniazdo, (SockAddr*)&adres, &i) == -1) {
        //     perror("Serwer: Blad funkcji getsockname.\n");
        //     close(gniazdo);
        //     return -1;
        // }

        // drukujemy info pomocnicze z numerem portu
        printf("Serwer: Nr portu : %d\n", ntohs(adres.sin_port));
        // nasluchujemy na poloczenie od klienta podajac gniazdo i limit klientow
        if (listen(gniazdo, 5) == -1) {
            perror("Serwer: Blad nasluchu gniazda.\n");
            close(gniazdo);
            return -1;
        }

        for (;;) {
            // akceptujemy polaczenie od clienta i zapisujemy jego adres w strukturze adres_k i zwracamy deskryptor gniazda klienta
            if ((gniazdo1 = accept(gniazdo, (SockAddr*)&adres_k, &i)) == -1) {
                perror("Serwer: Blad funkcji accept.\n");
                continue;
            }
            else {
                pid_t pid;
                // rozdzielamy proces
                if ((pid = fork()) == -1) {
                    perror("Serwer: Wywolanie funkcji fork nie powiodlo sie.\n");
                    close(gniazdo1);
                    close(gniazdo);
                    return -1;
                }

                if (pid == 0) {
                    ssize_t liczba_bajtow;
                    // w procesie potomnym zamykamy nie potrzebne juz gniazdo serwera
                    close(gniazdo);
                    // drukujemy info o klienciena podstawie zapisanego adresu
                    printf("Serwer, proces potomny: Polaczenie od %s z portu %d.\n", inet_ntoa(adres_k.sin_addr), ntohs(adres_k.sin_port));
                    // czytamy numer procesu klienta
                    if ((liczba_bajtow = read(gniazdo1, &pid, sizeof(pid_t))) == -1) {
                        perror("Serwer, proces potomny: Blad funkcji read.\n");
                        close(gniazdo1);
                        return -1;
                    }
                    // drukujemy info w postaci nr procesu klienta
                    printf("Serwer, proces potomny: Przeczytalem %ld bajtow od klienta, wiadomosc od klienta : %d.\n", liczba_bajtow, pid);
                    // zamykamy gniazdo procesu potomnego
                    close(gniazdo1);
                    return 0;
                }

                close(gniazdo1);
            }
        }
    }
}
