/*
Serwer oparty na kolejkach FIFO
*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<signal.h>
#include<string.h>

static const char* katalog = "./fifos/marekr-FIFO-TMP-SERWER";
int f, f3;

// funkcja obsluguje sygnaly SIGTERM I SIGINT zamykajac otwarte strumienie
void ala(int i) {
    //zamyka strumienie do fifo "marekr-FIFO-TMP-SERWER"
    close(f);
    close(f3);
    // usuwa fifo "marekr-FIFO-TMP-SERWER"
    unlink(katalog);
    exit(0);
}

int main() {
    int f1, f2;
    char bufor[1000], * pom = bufor;
    pid_t nr_proc, pid;

    // obsluga sygnalow wymuszajacych zakonczenie programu
    signal(SIGTERM, ala); 
    signal(SIGINT, ala);

    // usuwa fifo "marekr-FIFO-TMP-SERWER" jesli juz istnieje
    // jesli fifo istnieje proba utworzenia nowego zwroci error
    unlink(katalog);

    // tworzenie fifo "marekr-FIFO-TMP-SERWER";
    if (mkfifo(katalog, 0666) == -1) {
        perror("Serwer: Blad tworzenia kolejki FIFO. [1]\n");
        return -1;
    }
    
    // otwarcie strumieni zapisu i czytania do "marekr-FIFO-TMP-SERWER"
    // trzeba otworzyc strumien zapisu mimo iz go nie uzywamy zeby utrzymac blokowanie odczytu gdy klient zamknie swoj strumien zapisu - SIGPIPE/IO error!
    if ((f = open(katalog, O_RDONLY)) == -1 || (f3 = open(katalog, O_WRONLY)) == -1) {  
        perror("Serwer: Blad otwarcia kolejki FIFO. [2]\n");
        return -1;
    }

    // nieskonczona petla
    for (;;) {
        // czyta nr procesu z "marekr-FIFO-TMP-SERWER"
        read(f, &nr_proc, sizeof(pid_t));
        // tworzy proces potomny aby rozne zapytania odbywaly sie w sposob asynchroniczny
        if ((pid = fork()) == -1) {
            perror("Wywolanie funkcji fork nie powiodlo sie.\n");
            return -1;
        }
        // dalsze dzialania odbywaja sie w procesach potomnych aby kazde zapytanie do serwera bylo realizowane w innym procesie niz macierzysty
        if (pid == 0) {
            // ponizsze 3 linijki tworza sciezke do fifo "marekr-FIFO-TMP-2-83700" sluzacego odczytowi 
            strcpy(pom, "./fifos/marekr-FIFO-TMP-2-");
            pom += strlen(pom);
            sprintf(pom, "%d", (int)nr_proc);
            // otwieramy strumin odczytu z fifo "marekr-FIFO-TMP-2-83700"
            if ((f1 = open(bufor, O_RDONLY)) == -1) /* kolejka do czytania */ {
                perror("Serwer: Blad otwarcia kolejki FIFO. [3]\n");
                close(f);
                return -1;
            }
            // zmieniamy "marekr-FIFO-TMP-2-83700" na "marekr-FIFO-TMP-1-83700"
            *(pom - 2) = '1';
            // otwieramy strumien zapisu w fifo "marekr-FIFO-TMP-1-83700" 
            if ((f2 = open(bufor, O_WRONLY)) == -1) /* kolejka do pisania */ {
                perror("Serwer: Blad otwarcia kolejki FIFO. [4]\n");
                close(f1);
                close(f);
                return -1;
            }

            double a, b;
            // czytamy strumieniem f1 z fifo "marekr-FIFO-TMP-2-83700" wartosc i zapisuja ja w zmiennej a
            read(f1, &a, sizeof(double));
            b = a * a;
            // zapisujemy wartosc b strumieniem f2 do fifo "marekr-FIFO-TMP-1-83700" 
            write(f2, &b, sizeof(double));

            // nie zamykamy strumienia f3 z uwagi na wczesniejszy opis
            close(f1);
            close(f2);
            close(f);

            return 0;
        }
        // po co sigignore(SIGCHLD) program wydaje sie dzialac poprawnie bez tego?
        //sigignore(SIGCHLD);
    }
}

// Permissions:
// 1 – can execute
// 2 – can write
// 4 – can read

// The octal number is the sum of those free permissions, i.e.
// 3 (1+2) – can execute and write
// 6 (2+4) – can write and read

// Position of the digit in value:
// 1 – what owner can
// 2 – what users in the file group(class) can
// 3 – what users not in the file group(class) can

// Examples:
// chmod 600 file – owner can read and write
// chmod 700 file – owner can read, write and execute
// chmod 666 file – all can read and write
// chmod 777 file – all can read, write and execute