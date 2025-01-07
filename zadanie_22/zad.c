#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

typedef struct stru
{
    char* bufor;
    FILE* file;
    int* czytaj;
    pthread_rwlock_t* rygiel;
}Dane;

void* pisarz(void* arg)
{
    Dane* dane = (Dane*)arg;
    size_t len = 0;
    ssize_t znaki;
    char* linia = NULL;
    char* pomoc = dane->bufor;
    int t;
    srand(time(NULL));
    while ((znaki = getline(&linia, &len, dane->file)) != -1)
    {
        pthread_rwlock_wrlock(dane->rygiel);
        strcpy(pomoc, linia);
        pthread_rwlock_unlock(dane->rygiel);
        printf("-------------------------------------------dodalem linie\n");
        t = (rand() % 4) + 1;
        sleep(t);
        pomoc += znaki;
    }
    t = (rand() % 4) + 1;
    sleep(t);
    *(dane->czytaj) = 0;

    free(linia);
    return arg;
}

void* czytelnik(void* arg)
{
    Dane* dane = (Dane*)arg;
    int t;

    while (*(dane->czytaj))
    {
        pthread_rwlock_rdlock(dane->rygiel);
        printf("%s\n", dane->bufor);
        pthread_rwlock_unlock(dane->rygiel);
        srand(time(NULL));
        t = (rand() % 4) + 1;
        sleep(t);
    }
    return arg;
}

int main(int argc, char** argv)
{
    int i, czytaj = 1;
    FILE* file;
    char* sciezka = strndup(argv[1], strlen(argv[1]));
    size_t m = atoi(argv[2]);
    Dane* dane = (Dane*)malloc(sizeof(Dane));
    pthread_t* watki = (pthread_t*)malloc((sizeof(pthread_t) * m) + 1);
    pthread_rwlock_t kontrola = PTHREAD_RWLOCK_INITIALIZER;

    if ((file = fopen(sciezka, "r")) == NULL)
    {
        perror("fopen()\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    dane->bufor = (char*)malloc(sizeof(char) * ftell(file));
    fseek(file, 0, SEEK_SET);
    dane->czytaj = &czytaj;
    dane->file = file;
    dane->rygiel = &kontrola;

    for (i = 0; i <= m; i++)
    {
        if (i != m)
            pthread_create(&(watki[i]), NULL, czytelnik, (void*)dane);
        else
            pthread_create(&(watki[i]), NULL, pisarz, (void*)dane);
    }

    for (i = 0; i <= m; i++)
        pthread_join(watki[i], NULL);

    fclose(file);
    free(sciezka);
    free(dane->bufor);
    free(dane);
    free(watki);

    return 0;
}
