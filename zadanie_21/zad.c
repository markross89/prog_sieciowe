#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

typedef struct stru
{
    double* tablica;
    size_t dlugosc;
    double* wynik;
}Dane;

void* obsluga(void* arg)
{
    Dane* tab = (Dane*)arg;
    int dlugosc = tab->dlugosc, i;

    for (i = 0; i < dlugosc; i++)
        *tab->wynik += (tab->tablica[i] * tab->tablica[i]);

    free(tab);
    return NULL;
}

int main(int agrc, char* argv[])
{
    size_t n = atoi(argv[1]);
    int i, j;
    Dane* dane;

    double* wyniki = (double*)malloc(sizeof(double) * n);
    pthread_t* watki = (pthread_t*)malloc(sizeof(pthread_t) * n); 
    double** tablica = (double**)malloc(sizeof(double*) * n);
    tablica[0] = (double*)malloc(sizeof(double) * n * n);

    for (i = 1; i < n; i++)
        tablica[i] = &(tablica[0][i * n]);

    srand(time(NULL));
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            tablica[i][j] = (rand() % 1000) / 77;
            printf("[%d][%d] = %lf,   ", i, j, tablica[i][j]);
        }
        printf("\n");
    }

    for (i = 0; i < n; i++)
    {
        dane = (Dane*)malloc(sizeof(Dane));
        dane->tablica = tablica[i];
        dane->dlugosc = n;
        dane->wynik = &wyniki[i];
        pthread_create(&(watki[i]), NULL, obsluga, (void*)dane);
    }
    for (i = 0; i < n; i++)
        pthread_join(watki[i], NULL);
    
    double max = wyniki[0];
    for (i = 0; i < n; i++)
    {
        printf("Suma kwadratow elementow wiersza pierwszego: %lf\n", wyniki[i]);
        if (max < wyniki[i]) max = wyniki[i];
    }

    printf("Maksymalna wartosc z sum kwadratow kazdego wiersza: %lf\n", max);

    free(tablica[0]);
    free(tablica);
    free(watki);
    free(wyniki);

    return 0;
}