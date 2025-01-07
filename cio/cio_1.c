/*
Przyklad programu zapisujacego dane do pliku w trybie blokowanym.
*/

#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
    int liczbaIteracji = 1, i;
    char buf[1024];
    size_t size = 0;

    printf("Program testowy.\n");
    if (argc > 1) {

        /* konwersja pierwszego parametru wywolania na typ int */
        liczbaIteracji = strtol(argv[1], NULL, 10);
    }

    /* otwarcie pliku do pisania w trybie blokowanym */
    int plik = open("testowy.txt", O_CREAT | O_WRONLY, 0644);

    for (i = 0; i < liczbaIteracji; i++) {

        /* tworzenie wiadomości do zapisania w pliku */
        sprintf(buf, "Zawartosc testowa, iteracja numer %d.\n", i);

        /* zapisanie wiadomości do pliku */
        size += write(plik, buf, strlen(buf));
    }

    /* komunikat z podsumowaniem liczby zapisanych bajtow do pliku */
    printf("Zapisalem do pliku %lu bajtow.\n", size);

    /* zamkniecie pliku */
    close(plik);
    return 0;
}