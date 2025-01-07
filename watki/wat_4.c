/*
Program wielowątkowy z synchronizacją obliczeń wszystkich wątków - wariant 1
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

typedef struct dane {
    // deklaracja wskaznikow na double
    double* tab, * maksimum;
    int dlugosc, * ilosci, liczba;
    // deklaracja mutex
    pthread_mutex_t* rygiel;
    // deklaracja warunku
    pthread_cond_t* warunek;
} Dane;

void* obsluga(void* x) {
    // rzutuje wskaznik void na typ dane
    Dane* a = (Dane*)x;
    int i;
    pthread_t id = pthread_self();
    double* wynik = (double*)malloc(sizeof(double));

    // wyszukuje najwieksza wartosc
    *wynik = a->tab[0];
    for (i = 1; i < a->dlugosc; i++)
        if (*wynik < a->tab[i]) *wynik = a->tab[i];

    printf("Przed synchronizacja watek %ld.\n", (long)id);
    // rygluje watek dla obliczen
    pthread_mutex_lock(a->rygiel);
    // zwieksza ilosc
    (*(a->ilosci))++;
    // jesli ilosc rowna jest liczbie to wyzeruj ilosc
    if (*(a->ilosci) == a->liczba) {
        *(a->ilosci) = 0;
        // wysyla sygnal do wszystkich zablokowanych procesow
        pthread_cond_broadcast(a->warunek);
    }
    else
    {
        // blokuje watek i zwalnia rygiel obliczen. Potem czeka na sygnal a po jego otrzymaniu rygluje watek dla obliczen i rusza raz jeszcze z tego miejsca
        // w tym przypadku watek zwieksza ilosc i sprawdza czy jest rowna liczbie. jesli nie to blokuje ten watek i odpala kolejny ktory wykonuje to samo
        // w momecie ktorym jakis watek zrowna ilosc z liczba, ilosc zostanie wyzerowana a broadcast wysle sygnal do wszystkich watkow zeby kontynuowaly
        pthread_cond_wait(a->warunek, a->rygiel);
    }
    // zwalnia watek
    pthread_mutex_unlock(a->rygiel);

    printf("Po synchronizacji watek %ld.\n", (long)id);
    // wyszukuje nowego maksimum z posrod wszystkich watkow
    if (*(a->maksimum) < *wynik) {
        // blokuje watek
        pthread_mutex_lock(a->rygiel);
        // zmienia wartosc jesli warunek spelniony
        if (*(a->maksimum) < *wynik) *(a->maksimum) = *wynik;
        // zwalnia watek
        pthread_mutex_unlock(a->rygiel);
    }
    free(a);
    // wynik musimy zwrocic jako wskaznik typu void
    return (void*)wynik;
}

int main(int argc, char** argv) {
    int m = atoi(argv[1]), n, i, j, zliczaj = 0;
    double** tablica, * pomoc, wynik;
    pthread_t* watki;
    pthread_mutex_t kontrola = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bariera = PTHREAD_COND_INITIALIZER;
    Dane* y;

    sscanf(argv[2], "%d", &n);
    // zamawiamy miejsce dla tablicy pierwszego wymiaru
    tablica = (double**)malloc(sizeof(double*) * m);
    // zamawiamy pamiec dla watkow
    watki = (pthread_t*)malloc(sizeof(pthread_t) * m);
    // zamawiamy duza pamiec dla tablicy drugiego wymiaru 
    tablica[0] = (double*)malloc(sizeof(double) * m * n);
    // dzielimy duza pamiec na podtablice 2 wymiaru
    for (i = 1; i < m; i++)
        tablica[i] = &(tablica[0][i * n]);
    srand(time(NULL));
    // wypelniamy tablice losowymi liczbami od 0 - 12 i drukujemy dane
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            tablica[i][j] = (rand() % 1000) / 77;
            printf("[%d][%d]=%lf ", i, j, tablica[i][j]);
        }
        printf("\n");
    }
    // przypisujemy pierwszy element dwuwymiarowej tablicy do zmiennej wynik
    wynik = tablica[0][0];
    // iterujemy przez wszystkie tablice
    for (i = 0; i < m; i++) {
        // zamawiamy pamiec dla struktury dane
        y = (Dane*)malloc(sizeof(Dane));
        y->liczba = m;
        y->dlugosc = n;
        y->tab = tablica[i];
        y->maksimum = &wynik;
        y->ilosci = &zliczaj;
        y->rygiel = &kontrola;
        y->warunek = &bariera;
        // tworzymy watek
        pthread_create(&(watki[i]), NULL, obsluga, (void*)y);
    }
    // odpalamy watek
    for (i = 0; i < m; i++) {
        pthread_join(watki[i], (void**)&pomoc);
        free(pomoc);
    }
    printf("Wynik %lf.\n", wynik);
    // zwalniamy pamiec watkow oraz tablicy dwuwymiarowej
    free(tablica[0]);
    free(watki);
    free(tablica);
    return 0;
}