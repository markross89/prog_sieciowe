/*
Klient oparty na kolejkach FIFO
*/

#include<unistd.h>
#include<stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>

static const char* katalog = "./marekr-FIFO-TMP-SERWER";

int main() {
    int f1, f2;
    double a, b;
    char bufor[1000], * pom = bufor;
    pid_t numer_procesu = getpid();

    if ((f1 = open(katalog, O_WRONLY)) == -1) {
        perror("Klient: Nie moge ustanowic polaczenia z serwerem. [1]\n");
        return -1;
    }

    printf("Podaj liczbe: ");
    scanf("%lf", &a);
    write(f1, &a, sizeof(double));
    close(f1);

    if ((f2 = open(katalog, O_RDONLY)) == -1) {
        perror("Klient: Nie moge ustanowic polaczenia z serwerem. [1]\n");
        return -1;
    }

    read(f2, &b, sizeof(double));
    printf("Wynik obliczen %lf^2 = %lf.\n", a, b);
    close(f2);

    return 0;
}

