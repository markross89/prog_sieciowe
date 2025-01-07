/*
Przykład programu zapisującego dane do pliku w trybie nieblokowanym.
*/

#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[]) {
    int liczbaIteracji = 1, i;
    char buf[1024];
    size_t size = 0;

    printf("Program testowy.\n");
    if (argc > 1) {

        /* konwersja pierwszego parametru wywolania na tym int */
        liczbaIteracji = strtol(argv[1], NULL, 10);
    }

    /* otwarcie pliku do pisania w trybie nieblokowanym */
    int plik = open("testowy.txt", O_CREAT | O_WRONLY | O_NONBLOCK, 0644);

    for (i = 0; i < liczbaIteracji; i++) {

        /* tworzenie wiadomości do zapisania w pliku */
        sprintf(buf, "Zawartosc testowa, interacja numer %d.\n", i);

        /* zapisanie wiadomości do pliku */
        size += write(plik, buf, strlen(buf));
    }

    /* komunikat z podsumowaniem liczby zapisanych bajtow do pliku */
    printf("Zapisalem do pliku %lu bajtow.\n", size);

    /* zamkniecie pliku */
    close(plik);
    return 0;
}