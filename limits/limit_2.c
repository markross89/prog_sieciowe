/*
Pobranie i ustawienie wielkosci segmentu danych dla procesu. Polecenia
getrlimit i setrlimit sluza do pobrania i ustawiania roznych limitow
w systemie operacyjnym. Nie na wszystkich systemach operacyjnych program
powoduje blad z powodu przekrocenia limitu segmentu danych dla procesu,
efekt działania programu zalezy od konfiguracji systemu operacyjnego.
Program ma problemy z poprawna praca na wszelkich emulatorach systemow
typu uniksowego.
*/

#include<stdio.h>
#include<sys/resource.h>
#include<stdlib.h>

int main(int argc) {
    struct rlimit pomoc;
    char* p1, * p2;

    getrlimit(RLIMIT_AS, &pomoc);
    printf("soft: %lu, hard: %lu\n", pomoc.rlim_cur, pomoc.rlim_max);
    pomoc.rlim_cur = 6000;
    if (argc == 1) {
        setrlimit(RLIMIT_AS, &pomoc);
    }
    getrlimit(RLIMIT_AS, &pomoc);
    printf("soft: %lu, hard: %lu\n", pomoc.rlim_cur, pomoc.rlim_max);
    printf("Przydzielam pamiec dla zmiennej p1.\n");
    if (!(p1 = (char*)malloc(5000))) {
        perror("Brak pamieci dla zmiennej p1.\n");
        return -1;
    }
    printf("Przydzielam pamiec dla zmiennej p2.\n");
    if (!(p2 = (char*)malloc(7000))) {
        perror("Brak pamieci dla zmiennej p2.\n");
        return -1;
    }
    printf("Poprawnie zakonczylem przydzial pamieci.\n");
    return 0;
}