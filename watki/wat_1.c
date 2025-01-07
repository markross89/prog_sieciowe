#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

struct dane {
    double *tab;
    int dlugosc;
};
typedef struct dane Dane;

void *obsluga(void *x) {
    Dane *a = (Dane *) x;
    int i;
    double *wynik = (double *) malloc(sizeof(double));
    *wynik = a->tab[0];
    for (i = 1; i < a->dlugosc; i++)
        if (*wynik < a->tab[i]) *wynik = a->tab[i];
    free(a);
    return (void *) wynik;
}

int main(int argc, char **argv) {
    int m = atoi(argv[1]), n, i, j;
    double **tablica, *pomoc, wynik;
    // pointer na watki
    pthread_t *watki;
    Dane *y;
    sscanf(argv[2], "%d", &n);
    tablica = (double **) malloc(sizeof(double *) * m);
    // alokujemy pamiec dla watkow - po jednym na kazda tablice drugiego wymiaru
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
    // alokujemy pamiec dla danych z kazdej tablicy i tworzymy tyle samo watkow
    for (i = 0; i < m; i++) {
        y = (Dane *) malloc(sizeof(Dane));
        y->dlugosc = n;
        y->tab = tablica[i];
        pthread_create(&(watki[i]), NULL, obsluga, (void *) y);
    }
    // do wyniku przypisujemy pierwszy element z dwuwymiarowej struktury
    wynik = tablica[0][0];
    for (i = 0; i < m; i++) {
        // dla kazdej tablicy odpalamy watek i rezultat zapisujemy do zmiennej pomoc 
        pthread_join(watki[i], (void **) &pomoc);
        // sprawdzamy ktory wynik jest najwiekszy
        if (wynik < *pomoc) wynik = *pomoc;
        // zwalniamy pamiec dla pomocy
        free(pomoc);
    }
    // drukujemy
    printf("Wynik %lf.\n", wynik);
    // zwalniamy pamiec dla duzej tablicy drugiego wymiaru
    free(tablica[0]);
    // zwalniamy pamiec dla watkow
    free(watki);
    // zwalniamy pamiec dla tablicy pierwszego wymiaru
    free(tablica);
    return 0;
}