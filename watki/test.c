#include<stdio.h>
#include<stdlib.h>
#include<time.h>

struct dane {
    double *tab;
    int dlugosc;
};
typedef struct dane Dane;

// funkcja przyjmuje tablice o okreslonej dlugosci przez struct dane
double *obsluga1(Dane *a) {
    int i;
    //tworzy pointer wynik i alokuje pamiec
    double *wynik = (double *) malloc(sizeof(double));
    // przypisuje pierwszy element tablicy dane do wyniku
    *wynik = a->tab[0];
    // iteruje przez reszte elementow tablicy dane
    for (i = 1; i < a->dlugosc; i++)
        // jesli jakas wartosc tablicy dane jest wieksza od wyniku to jest to nowa wartosc wyniku
        if (*wynik < a->tab[i]) *wynik = a->tab[i];
    // zwalniamy pamiec alokowana dla zmiennej dane
    free(a);
    // zwracamy wynik
    return wynik;
}

int main(int argc, char **argv) {
    // zapisujemy pierwszy argument funkcji do zmiennej m jako pierwszy wymiar tablicy
    int m = atoi(argv[1]), n, i, j;
    // deklarujemy zmienne w tym dwuwymiarowom tablice
    double **tablica, *pomoc, wynik;
    // deklarujemy strukture dane
    Dane *y;
    // przypisujemy drugi argument do zmiennej n jako drugi wymiar tablicy
    sscanf(argv[2], "%d", &n);
    // zamawiamy pamiec dla m tablic
    tablica = (double **) malloc(sizeof(double *) * m);
    // zamawiamy pamiec tablicy dla m * n (jedna wielka tablica wszystkich tablic drugiego wymiaru)
    tablica[0] = (double *) malloc(sizeof(double) * m * n);
    // dzielimy wielka tablice na podtablice drugiego wymiary
    for (i = 1; i < m; i++)
        tablica[i] = &(tablica[0][i * n]);
    srand(time(NULL));
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            // wypelniamy tablice losowymi liczbami od 0 - 12 
            tablica[i][j] = (rand() % 1000) / 77;
            // drukujemy
            printf("[%d][%d]=%lf ", i, j, tablica[i][j]);
        }
        printf("\n");
    }
    // alokujemy pamiec dla danych
    y = (Dane *) malloc(sizeof(Dane));
    // uzupelniamy dane o dlugosc tablicy 2 wymiaru
    y->dlugosc = n;
    // tabice 0
    y->tab = tablica[0];
    // wywolujemy funkcje obsluga 1 i zwracamy pointer do najwiekszego elementu tablicy 0
    pomoc = obsluga1(y);
    // drukujemy wynik
    printf("Wynik %lf.\n", *pomoc);
    // zwalniamy pamiec jednej wielkiej tablicy drugiego wymiaru
    free(tablica[0]);
    // zwalniamy pamiec tablicy pierwszego wymiaru
    free(tablica);
    return 0;
}