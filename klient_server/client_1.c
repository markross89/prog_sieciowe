/*
Klient oparty na kolejkach FIFO
*/

#include<unistd.h>
#include<stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>

static const char* katalog = "./fifos/marekr-FIFO-TMP-SERWER";

int main() {
    pid_t numer_procesu;
    int f, f1, f2;
    char bufor[1000], * pom = bufor;
    numer_procesu = getpid();

    // otwieramy strumien do 'marekr-FIFO-TMP-SERWER' ktory juz istnieje i czeka na zapis
    if ((f = open(katalog, O_WRONLY)) == -1) {
        perror("Klient: Nie moge ustanowic polaczenia z serwerem. [1]\n");
        return -1;
    }
    // zapisujemy "marekr-FIFO-TMP-1-" w buforze przez wskaznik pom
    strcpy(pom, "./fifos/marekr-FIFO-TMP-1-");  
    // przesuwamy wskaznik pom na koniec wyrazenia "marekr-FIFO-TMP-1-"
    pom += strlen(pom);
    // dopisujemy numer procesu do bufora otrzymujac np. "marekr-FIFO-TMP-1-83700"
    sprintf(pom, "%d", (int)numer_procesu);

    // tworzymy fifo "marekr-FIFO-TMP-1-83700" do czytania 
    if (mkfifo(bufor, 0600) == -1) /* kolejka do czytania */ {
        perror("Klient: Blad tworzenia kolejki FIFO. [2]\n");
        close(f);
        return -1;
    }

    // przesuwamy wskaznik ustawiony na koniec wyrazenia "marekr-FIFO-TMP-1-" o 2 do tylu i zmieniamy 1 na 2 dostajac "marekr-FIFO-TMP-2-83700"
    *(pom - 2) = '2';

    // // tworzymy fifo "marekr-FIFO-TMP-2-83700" do pisania 
    if (mkfifo(bufor, 0600) == -1) /* kolajka do pisania */ {
        perror("Klient: Blad tworzenia kolejki FIFO. [3]\n");
        close(f);
        return -1;
    }

    // zapisujemy strumieniem na "marekr-FIFO-TMP-SERWER" numer obecnego procesu
    write(f, &numer_procesu, sizeof(pid_t));
    // zamykamy niepotrzebny juz strumien
    close(f);

    // otwieramy strumien do zapisu w "marekr-FIFO-TMP-2-83700"
    if ((f1 = open(bufor, O_WRONLY)) == -1) /* kolejka do pisania */ {
        perror("Klient: Blad otwarcia kolejki FIFO. [4]\n");
        close(f);
        return -1;
    }

    *(pom - 2) = '1';
    // otwieramy strumien do czytania z "marekr-FIFO-TMP-1-83700"
    if ((f2 = open(bufor, O_RDONLY)) == -1) /* kolajka do czytania */ {
        perror("Klient: Blad otwarcia kolejki FIFO. [5]\n");
        close(f1);
        close(f);
        return -1;
    }

    double a, b;

    printf("Podaj liczbe: ");
    scanf("%lf", &a);
    // zapisujemy wartosc a strumieniem f1 w "marekr-FIFO-TMP-2-83700"
    write(f1, &a, sizeof(double));
    // czytamy wartosc strumieniem f2 z "marekr-FIFO-TMP-1-83700" i zapisujemy w zmiennej b
    read(f2, &b, sizeof(double));
    printf("Wynik obliczen %lf^2 = %lf.\n", a, b);

    close(f1);
    close(f2);
    close(f);

    // usuwamy fifo "marekr-FIFO-TMP-1-83700"
    unlink(bufor);
    // zmieniamy zawartos buora na "marekr-FIFO-TMP-2-83700"
    *(pom - 2) = '2';
    // usuwamy fifo "marekr-FIFO-TMP-2-83700"
    unlink(bufor);

    return 0;

}