/*
Klient strumieniowy, internetowy.
*/

#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>

#define h_addr h_addr_list[0]
// struktura adresu w strukturze adresu internetowego
typedef struct sockaddr SockAddr;
// struktura adresu internetowego
typedef struct sockaddr_in SockAddr_in;

int main(int argc, char* argv[]) {
    int gniazdo;
    socklen_t i;
    SockAddr_in adres;
    int czy_czekac = 0, liczba_sekund = 0;
    pid_t pid;

    if (argc < 3) {
        printf("Klient: Bledna ilosc argumentow (adres internetowy numer portu [czas oczekiwania na wyslanie]).\n");
        return -1;
    }

    if (argc > 3) {
        czy_czekac = 1;
        liczba_sekund = strtol(argv[3], NULL, 10);
    }
    pid = getpid();
    // otwieramy gniazdo, podajac rodzine protokolow(np. unix[AF_UNIX], ipv4[AF_INET], ipv6[AF_INET6]) 
    // i protokul komunikacyjny(np. tcp[SOCK_STREAM]{FULL DUPLEX}, udp(SOCK_DGRAM), raw(SOCK_RAW))
    if ((gniazdo = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Klient: Blad wywolania funkcji socket.\n");
        return -1;
    }

    // struct hostent {
    //     char* h_name;            /* official name of host */
    //     char** h_aliases;         /* alias list */
    //     int    h_addrtype;        /* host address type */
    //     int    h_length;          /* length of address */
    //     char** h_addr_list;       /* list of addresses */
    // }
    // #define h_addr h_addr_list[0] /* for backward compatibility */
    struct hostent* Adres;

    i = sizeof(SockAddr_in);
    adres.sin_family = AF_INET;
    adres.sin_port = htons(strtol(argv[2], NULL, 10));
    // zapisanie informacji o hoscie w strkturze Adres
    // The gethostbyname*(), gethostbyaddr*(), herror(), and hstrerror() functions are obsolete.  Applications should use getaddrinfo(3), getnameinfo(3), and gai_strerror(3) instead.
    Adres = gethostbyname(argv[1]);
    // podbranie adresu z structu Adres
    adres.sin_addr.s_addr = *(long*)(Adres->h_addr);
    // polaczenie z serwerem podajac gniazdo, adres internetowy kastowany na adres i wielkkosc struktury adres internetowy
    if (connect(gniazdo, (SockAddr*)&adres, i) == -1) {
        perror("Klient: Blad funkcji connect.\n");
        close(gniazdo);
        return -1;
    }

    if (czy_czekac) {
        printf("Klient: Oczekuje %d sekund na wyslanie wiadomosci do serwera.\n", liczba_sekund);
        sleep(liczba_sekund);
    }
    printf("Klient: Klient wysyla pid %d.\n", pid);
    // zapisujemy do gniazda numer procesu
    if (write(gniazdo, &pid, sizeof(pid_t)) == -1) {
        perror("Klient: Blad funkcji write.\n");
        close(gniazdo);
        return -1;
    }
    // zamykamy gniazdo
    close(gniazdo);
    return 0;
}