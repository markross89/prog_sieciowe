#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


struct dane {
    double *tab;
    int liczba_iteracji;
    // deklaracja watku
    pthread_t *watek;
    // deklaracja mutex zapisu i odczytu
    // rygiel zapisu dziala jak mutex ale rygiel odczytu pozwala na odczyt wielu odbiorcom w tym samym czasie
    pthread_rwlock_t *rygiel;
};
typedef struct dane Dane;

// obsluga zapisu
void *obsluga_p(void *x) {
    // pointer do danych przekazywany jest w formie void * a wiec castujemy go do wlasciwego typu
    Dane *a = (Dane *) x;
    int i;
    // zwraca id watku
    pthread_t id = pthread_self();
    for (i = 0; i < a->liczba_iteracji; i++) {
        // blokada watku zapisu
        pthread_rwlock_wrlock(a->rygiel);
        // zapisanie id watku w danych
        (*(a->watek)) = id;
        // dodanie wartosci i do wartosci tab
        (*(a->tab)) += i;
        printf("Watek %d zmodyfikowal na %lf.\n", (int)id, *(a->tab));
        // zwalniamy blokade
        pthread_rwlock_unlock(a->rygiel);
        sleep(5);
    }
    // zwalniamy pamiec dla a zadeklarowana w funkcji main
    free(a);
    return NULL;
}

// obsluga czytania
void *obsluga_cz(void *x) {
    Dane *a = (Dane *) x;
    int i;
    pthread_t id = pthread_self();
    for (i = 0; i < a->liczba_iteracji; i++) {
        // blokujemy rygiel odczytu
        pthread_rwlock_rdlock(a->rygiel);
        printf("Watek %d przeczytal %lf zapisane przez %d.\n", (int)id, *(a->tab), (int)*(a->watek));
        // zwalniamy rygiel 
        pthread_rwlock_unlock(a->rygiel);
        sleep(2);
    }
    // zwalniamy pamiec zmowiona w funkcji main
    free(a);
    return NULL;
}

int main(int argc, char **argv) {
/* m - liczba pisarzy, n - liczba czytelnikow, l - liczba iteracji pisarzy, k - liczba iteracji czytelnikow */
    int m = atoi(argv[1]), n, i, l, k;
    double pomoc = 3.33;
    pthread_t *watki_p, *watki_cz, watek;
    // inicjalizujemy mutex zapisu i odczytu
    pthread_rwlock_t kontrola = PTHREAD_RWLOCK_INITIALIZER;
    Dane *y;
    sscanf(argv[2], "%d", &n);
    sscanf(argv[3], "%d", &l);
    sscanf(argv[4], "%d", &k);
    // zamawiamy pamiec dla watkow pisarzy
    watki_p = (pthread_t *) malloc(sizeof(pthread_t) * m);
    // zamawiamy pamiec dla watkow czytelnikow
    watki_cz = (pthread_t *) malloc(sizeof(pthread_t) * n);

    // obslugujemy pisarzy
    for (i = 0; i < m; i++) {
        // zamawiamy pamiec dla struktury dane
        y = (Dane *) malloc(sizeof(Dane));
        // ustawiamy wartosci
        y->watek = &watek;
        y->tab = &pomoc;
        y->rygiel = &kontrola;
        y->liczba_iteracji = l;
        // tworzymy watek
        pthread_create(&(watki_p[i]), NULL, obsluga_p, (void *) y);
    }
    // obslugujemy czytelnikow
    for (i = 0; i < n; i++) {
        y = (Dane *) malloc(sizeof(Dane));
        y->watek = &watek;
        y->tab = &pomoc;
        y->rygiel = &kontrola;
        y->liczba_iteracji = k;
        pthread_create(&(watki_cz[i]), NULL, obsluga_cz, (void *) y);
    }
    // odpalamy watki pisarzy
    for (i = 0; i < m; i++)
        pthread_join(watki_p[i], NULL);
    // odpalamy watki czytelnikow
    for (i = 0; i < n; i++)
        pthread_join(watki_cz[i], NULL);

    // zwlaniamy pamiec watkow - pamiec struktur zwolniona w obslugach watkow    
    free(watki_p);
    free(watki_cz);
    return 0;
}

// ./wat_3 2 5 2 4  
// odpali 2 watki pisarzy ktorzy zablokuja odczyt i niezmienia wartosci bo zmienna i w obsludze pisarzy = 0 w pierwszej iteracji - potem czeka 5 sekund 
// w tym czasie odpala sie 5 watkow czytelnikow ktorzy iteruja 4 razy i czytaja dane. w ciagu 5 sekund wydrukuja 15 linijek i zostana zablokowane 
// druga iteracja watku pisarzy zostaje odpalona po 5 sekundach i kazdy z pisarzy inkrementuje wartosc tab jeden raz
// na koniec czytelnicy odczytaja ostatnia iteracje czyli 5 linijek