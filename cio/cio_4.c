/*
Program kopiuje zawartosc jednego pliku na drugi z zadana wielkoscia bufora.
*/

#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[]) {
    int plik, plik2;
    void* bufor;
    size_t rozmiar, rozmiar1;

    if (argc != 4) {
        perror("Bledna ilosc argumentow\n");
        return -1;
    }
    plik = open(argv[1], O_RDONLY);
    if (plik == -1) {
        perror("Blad otwarcia pliku do czytania");
        return -1;
    }
    plik2 = open(argv[2], O_WRONLY | O_CREAT, 0644);
    if (plik2 == -1) {
        perror("Blad otwarcia pliku do pisania.\n");
        return -1;
    }
    rozmiar = strtol(argv[3], NULL, 10);
    if (rozmiar <= 0) {
        perror("Bledny rozmiar bufora.\n");
        return -1;
    }
    if (!(bufor = (void*)malloc(rozmiar))) {
        perror("Blad przydzialu pamieci.\n");
        return -1;
    }
    while ((rozmiar1 = read(plik, bufor, rozmiar))) write(plik2, bufor, rozmiar1);
    return 0;
}
