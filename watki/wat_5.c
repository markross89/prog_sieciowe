/*
Program wielowątkowy z synchronizacją obliczeń wszystkich wątków - wariant 2
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

// definiuje strukture danych
typedef struct dane {
    double *tab, *maksimum;
    int dlugosc, liczba;
    pthread_mutex_t *rygiel;
} Dane;

// tworze warunek 
int bariera(int liczba) {
    static int ilosci = 0;
    static pthread_mutex_t rygiel = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t warunek = PTHREAD_COND_INITIALIZER;
    
    // rygluje watek dla obliczen
    pthread_mutex_lock(&rygiel);
    // zwiekszam ilosc
    ilosci++;
    // jesli ilosc zrowna sie z liczba to resetuje ilosc do 0
    if (ilosci == liczba) {
        ilosci = 0;
        // wysylam sygnal do wszystkich watkow zeby je odblokowac
        pthread_cond_broadcast(&warunek);
    } else 
    {
        // blokuje watek, zwalnia rygiel i czeka na sygnal ktory ponownie zarygluje watek dla obliczen i sprawdzi warunek
        pthread_cond_wait(&warunek, &rygiel);
    }
    // zwalniam rygiel obliczen
    pthread_mutex_unlock(&rygiel);
    return 0;
}

void *obsluga(void *x) {
    Dane *a = (Dane *) x;
    int i;
    pthread_t id = pthread_self();
    double *wynik = (double *) malloc(sizeof(double));

    // wyszukujemy najwiekszy element
    *wynik = a->tab[0];
    for (i = 1; i < a->dlugosc; i++)
        if (*wynik < a->tab[i]) *wynik = a->tab[i];
    printf("Przed synchronizacja watek %ld.\n", (long)id);
    // sprawdzamy warunek
    bariera(a->liczba);
    printf("Po synchronizacji watek %ld.\n", (long)id);
    // sprawdzamy czy zmienna maksimum przypisana jest do najwiekszego elementu i jesli nie to przypisujemy jej go
    if (*(a->maksimum) < *wynik) {
        // ryglujemy watek dla obliczen
        pthread_mutex_lock(a->rygiel);
        if (*(a->maksimum) < *wynik) *(a->maksimum) = *wynik;
        // zwalniamy rygiel
        pthread_mutex_unlock(a->rygiel);
    }
    free(a);
    return (void *) wynik;
}

int main(int argc, char **argv) {
    int m = atoi(argv[1]), n, i, j;
    double **tablica, *pomoc, wynik;
    pthread_t *watki;
    // inicjalizujemy mutex
    pthread_mutex_t kontrola = PTHREAD_MUTEX_INITIALIZER;
    Dane *y;

    sscanf(argv[2], "%d", &n);
    tablica = (double **) malloc(sizeof(double *) * m);
    watki = (pthread_t *) malloc(sizeof(pthread_t) * m);
    tablica[0] = (double *) malloc(sizeof(double) * m * n);
    for (i = 1; i < m; i++)
        tablica[i] = &(tablica[0][i * n]);
    srand(time(NULL));
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            tablica[i][j] = (rand() % 1000) / 77;
            printf("[%d][%d]=%lf ", i, j, tablica[i][j]);
        }
        printf("\n");
    }
    // inicjalizujemy watki
    wynik = tablica[0][0];
    for (i = 0; i < m; i++) {
        y = (Dane *) malloc(sizeof(Dane));
        y->liczba = m;
        y->dlugosc = n;
        y->tab = tablica[i];
        y->maksimum = &wynik;
        y->rygiel = &kontrola;
        // tworzymy nowy watek ktory wykona funkcje obsluga w ktorej z kolei sprawdzimy warunek
        pthread_create(&(watki[i]), NULL, obsluga, (void *) y);
    }
    // odpalamy watki
    for (i = 0; i < m; i++) {
        // po odpaleniu watek wykona obsluge ktora zwroci wskaznik i zapisze go w zmiennej wskaznikowej pomoc
        pthread_join(watki[i], (void **) &pomoc);
        // po zwroceniu pomoc wskazuje na to samo miejsce w pamieci co wynik w funkcji obsluga. w obsludze zmowilismy pamiec wiec musimy ja teraz zwolnic
        free(pomoc);
    }
    // drukujemy zmienna wynik ktora zostala przypisana do y->maksimum. zmienna wskaznikowa lokalna wynik w obsludze przypisuje wartosc do y->maksimum
    printf("Wynik %lf.\n", wynik);
    // zwalniam pamiec tablicy dwuwymiarowej oraz watkow
    free(tablica[0]);
    free(watki);
    free(tablica);
    return 0;
}