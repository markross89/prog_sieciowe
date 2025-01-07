#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<math.h>

typedef struct dane
{
    double p, * z;
    long m, n;
    pthread_cond_t* warunek;
    pthread_mutex_t* blokada;
}Dane;

double random_double(int max, int min)
{
    double random = ((double)rand()) / RAND_MAX;
    random *= max - min;
    random += min;
    return random;
}

void* obsluga(void* arg)
{
    Dane* dane = (Dane*)arg;
    double s;
    int i = 0;
    pthread_t watek = pthread_self();

    pthread_mutex_lock(dane->blokada);
    for (i = 0; i < dane->n; i++)
    {
        s = random_double(2, 1);
        *dane->z += s * dane->p;
        if (*dane->z > dane->m)
        {
            dane->p = *dane->z * (1.0 / ((double)dane->n));
            printf("%d - petla, wartosc z = %lf, watek %d, koczy petle i budzi wszystkie watki\n", i+1, *dane->z, (int)watek);
            *dane->z = 0;
            pthread_cond_broadcast(dane->warunek);
        }
        else
        {
            printf("%d - petla, wartosc z = %lf, watek %d, budzi inny watek\n", i+1, *dane->z, (int)watek);
            pthread_cond_wait(dane->warunek, dane->blokada);
        }
    }
    pthread_mutex_unlock(dane->blokada);
    printf("koniec pracy watku %d\n", (int)watek);

    return arg;
}

int main(int argc, char* argv[])
{
    srandom(time(NULL));
    int i;
    long n = strtol(argv[1], NULL, 0);
    long m = strtol(argv[2], NULL, 0);
    double p = strtod(argv[3], NULL);
    double* z = (double*)malloc(sizeof(double));
    *z = 0;
    Dane* dane = (Dane*)malloc(sizeof(Dane));
    pthread_t* watki = (pthread_t*)malloc(sizeof(pthread_t) * n);
    pthread_mutex_t blokada = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t warunek = PTHREAD_COND_INITIALIZER;

    dane->m = m;
    dane->p = p;
    dane->n = n;
    dane->z = z;
    dane->blokada = &blokada;
    dane->warunek = &warunek;

    for (i = 0; i < n; i++)
        pthread_create(&(watki[i]), NULL, obsluga, (void*)dane);

    for (i = 0; i < n; i++)
        pthread_join(watki[i], NULL);

    free(z);
    free(dane);
    free(watki);
    return 0;
}