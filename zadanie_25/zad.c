#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

typedef struct stru
{
    size_t n;
    double* tablica;
    double* max;
    pthread_mutex_t* blokada;
}Dane;

void* obsluga(void* arg)
{
    Dane* dane = (Dane*)arg;
    int i;
    double suma;

    for (i = 0; i < dane->n; i++) 
        suma += (dane->tablica[i] * dane->tablica[i]);
    
    printf("Suma kadratow wiersza = %lf\n", suma);

    pthread_mutex_lock(dane->blokada);
    if (suma > *dane->max) *dane->max = suma;
    pthread_mutex_unlock(dane->blokada);

    return (void*)dane->max;
}

int main(int argc, char** argv)
{
    int i, j;
    size_t n = (size_t)strtol(argv[1], NULL, 0);
    double** tablica = (double**)malloc(sizeof(double*) * n);
    tablica[0] = (double*)malloc(sizeof(double) * n * n);
    for (i = 1; i < n; i++)
        tablica[i] = &(tablica[0][i * n]);

    pthread_t* watki = (pthread_t*)malloc(sizeof(pthread_t) * n);
    pthread_mutex_t* rygiel = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    Dane* dane = (Dane*)malloc(sizeof(Dane));
    double* max = (double*)malloc(sizeof(double));
    *max = 0;
    dane->max = max;
    dane->blokada = rygiel;
    dane->n = n;

    srand(time(NULL));
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            tablica[i][j] = (rand() % 1000) / 77;
            printf("[%d][%d] = %lf     ", i, j, tablica[i][j]);
        }
        printf("\n");
    }

    for (i = 0; i < n; i++) {
        dane->tablica = &(tablica[0][i]);
        pthread_create(&(watki[i]), NULL, obsluga, (void*)dane);
    }
    void* wynik;
    for (i = 0; i < n; i++)
        pthread_join(watki[i], &wynik);
    
    printf("maksymalna wartosc sumy kwadratow wynosi: %lf\n", *(double*)wynik);

    free(max);
    free(tablica[0]);
    free(tablica);
    free(watki);
    return 0;
}