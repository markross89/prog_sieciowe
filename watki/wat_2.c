#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

struct dane {
    double *tab, *maksimum;
    int dlugosc;
    // deklaracja mutex
    pthread_mutex_t *rygiel;
};
typedef struct dane Dane;

void *obsluga(void *x) {
    Dane *a = (Dane *) x;
    int i;
    double *wynik = (double *) malloc(sizeof(double));
    *wynik = a->tab[0];
    for (i = 1; i < a->dlugosc; i++)
        if (*wynik < a->tab[i]) *wynik = a->tab[i];
        // sprawdzamy czy maksimum faktycznie jest maksimum
    if (*(a->maksimum) < *wynik) {
        // blokujemy mutex
        pthread_mutex_lock(a->rygiel);
        // zapisujemy nowa wartosc do maksimum
        if (*(a->maksimum) < *wynik) *(a->maksimum) = *wynik;
        // zwalniamy mutex
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
    // inna opcja nie posixowa to    pthred_mutex_init(&kontrola, NULL)
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
    wynik = tablica[0][0];
    for (i = 0; i < m; i++) {
        y = (Dane *) malloc(sizeof(Dane));
        y->dlugosc = n;
        y->tab = tablica[i];
        y->maksimum = &wynik;
        y->rygiel = &kontrola;
        pthread_create(&(watki[i]), NULL, obsluga, (void *) y);
    }
    for (i = 0; i < m; i++) {
        pthread_join(watki[i], (void **) &pomoc);
        free(pomoc);
    }
    printf("Wynik %lf.\n", wynik);
    free(tablica[0]);
    free(watki);
    free(tablica);
    return 0;
}