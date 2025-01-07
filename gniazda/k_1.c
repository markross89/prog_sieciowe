/*
Klient datagramowy, internetowy, wspolbiezny.
*/

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/un.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

typedef struct sockaddr SockAddr;
typedef struct sockaddr_in SockAddr_in;

int main(int argc, char* argv[]) {
    int gniazdo;
    socklen_t i1;
    in_port_t port;
    SockAddr_in adres_serw, adres_kli;
    pid_t pid;

    if (argc < 3) {
        printf("Bledna ilosc argumentow. (adres internetowy, numer portu) \n");
        return -1;
    }

    pid = getpid();
    // otwieram gniazdo
    if ((gniazdo = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Blad wywolania funkcji socket.\n");
        return -1;
    }

    char* s;
    struct hostent* Adres;
    // zeruje adres serwera
    bzero((char*)&adres_serw, sizeof(SockAddr_in));

    // ponizej ustawiam wartosc maksymalna klientow oraz id procesu oraz laduje je do zmiennej
    s = (char*)malloc(10 * sizeof(char));
    if (argc == 3) {
        sprintf(s, "%5d %d", pid, 5);
    }
    else {
        sprintf(s, "%5d %s", pid, argv[3]);
    }

    // ustawiam rodzine protokolow
    adres_serw.sin_family = AF_INET;
    // ustawiam port na podstawie podanego argumentu
    adres_serw.sin_port = htons(strtol(argv[2], NULL, 10));
    // ustawiam adres IP
    Adres = gethostbyname(argv[1]);
    // ustawiam adres dla struktury adresu internetowego przy pomocy structury adresu
    adres_serw.sin_addr.s_addr = *(long*)(Adres->h_addr);
    i1 = sizeof(SockAddr_in);

    // zeruje adres klienta
    bzero((char*)&adres_kli, sizeof(SockAddr_in));
    // rodzina protokolow ipv4
    adres_kli.sin_family = AF_INET;
    // adres IP 
    adres_kli.sin_addr.s_addr = htonl(INADDR_ANY);
    // port domyslny
    adres_kli.sin_port = htons(0);

    // przylaczam gniazdo klienta
    if (bind(gniazdo, (SockAddr*)&adres_kli, i1) < 0) {
        perror("Blad dowiazania gniazda.\n");
        close(gniazdo);
        return -1;
    }

    int pomoc;
    double argument;

    if (argc == 3) {
        pomoc = 2;
    }
    else {
        pomoc = strtol(argv[3], NULL, 10);
    }
    // wysylam dane do gniazda pid i maks numer procesow
    if (sendto(gniazdo, s, strlen(s), 0, (SockAddr*)&adres_serw, i1) < 0) {
        perror("Blad wywolania funkcji sendto.\n");
        close(gniazdo);
        return -1;
    }
    // zwalniam pamiec dla s
    free(s);
    // odbieram wiadomosc od serwera i zapisuje wartosc do zmiennej (nr portu -> &port), a takze adres serwera
    if (recvfrom(gniazdo, &port, sizeof(in_port_t), 0, (SockAddr*)&adres_serw, &i1) < 0) {
        perror("Blad wywolania funkcji recvfrom.\n");
        close(gniazdo);
        return -1;
    }
    // drukuje nr portu serwera
    printf("Numer portu serwera %d.\n", ntohs(port));
    // ustawiam nr portu serwera
    adres_serw.sin_port = port;
    // zamawiam pamiec dla zmiennej textowej
    s = (char*)malloc(25 * sizeof(char));
    // odbieram wiadomosc od serwera (czas) i zapisuje do zmiennej
    if (recvfrom(gniazdo, s, 25, 0, (SockAddr*)&adres_serw, &i1) < 0) {
        perror("Blad wywolania funkcji recvfrom.\n");
        close(gniazdo);
        return -1;
    }

    // w zaleznosci od zmiennej pomoc drukuje aktualny czas lub date
    if (pomoc == 1) {
        printf("Aktualny czas: %s.\n", s);
    }
    else {
        printf("Aktualna data: %s.\n", s);
    }
    // pobieram liczbe rzeczywista i wysylam ja na serwer
    printf("Podaj liczbe rzeczywista: ");
    scanf("%lf", &argument);
    if (sendto(gniazdo, &argument, sizeof(double), 0, (SockAddr*)&adres_serw, i1) < 0) {
        perror("Klient: Blad funkcji sendto.\n");
        close(gniazdo);
        return -1;
    }
    // odbieram liczbe rzeczywista z serwera 
    if (recvfrom(gniazdo, &argument, sizeof(double), 0, (SockAddr*)&adres_serw, &i1) < 0) {
        perror("Klient: Blad funkcji recfrom.\n");
        close(gniazdo);
        return -1;
    }
    // drukuje liczbe wynikowa
    printf("Wynik potegowania: %lf.\n", argument);
    
    // zwalniam pamiec 
    free(s);
    // zamykam gniazdo
    close(gniazdo);

    return 0;
}


