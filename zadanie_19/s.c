#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>
#include<semaphore.h>

const char* sciezka = "/Pamiec_wspolna";
const int len = 1024;
int pamiec_wspolna;
char* obszar;
sem_t* s1;
sem_t* s2;
sem_t* s3;

void wyczysc()
{
    sem_close(s1);
    sem_close(s2);
    sem_close(s3);
    sem_unlink("/semafor1");
    sem_unlink("/semafor2");
    sem_unlink("/semafor3");
    munmap(obszar, len);
    close(pamiec_wspolna);
    shm_unlink(sciezka);
    printf("Pamiec wyczyszczona\n");
}

void obsluga_sygnalow(int s)
{
    printf("\nKoniec pracy serwera\n");
    wyczysc();
    exit(0);
}

int main()
{
    signal(SIGTERM, obsluga_sygnalow);
    signal(SIGINT, obsluga_sygnalow);

    shm_unlink(sciezka);
    if ((pamiec_wspolna = shm_open(sciezka, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1)
    {
        perror("Serwer: shm_open");
        return 1;
    }
    if (ftruncate(pamiec_wspolna, len) == -1)
    {
        perror("Serwer: ftruncate");
        wyczysc();
        return -1;
    }
    if ((obszar = (char*)mmap(NULL, len, PROT_WRITE | PROT_READ, MAP_SHARED, pamiec_wspolna, 0)) == MAP_FAILED)
    {
        perror("Serwer: mmap");
        wyczysc();
        return -1;
    }

    if ((s1 = sem_open("/semafor1", O_RDWR | O_CREAT, 0666, 0)) == SEM_FAILED)
    {
        perror("Serwer: sem_open[1]");
        wyczysc();
        return -1;
    }

    if ((s2 = sem_open("/semafor2", O_RDWR | O_CREAT, 0666, 0)) == SEM_FAILED)
    {
        perror("Serwer: sem_open[2]");
        wyczysc();
        return -1;
    }

    if ((s3 = sem_open("/semafor3", O_RDWR | O_CREAT, 0666, 1)) == SEM_FAILED)
    {
        perror("Serwer: sem_open[3]");
        wyczysc();
        return -1;
    }


    printf("Serwer: %d\n", (int)getpid());
    for (;;)
    {
        sem_wait(s1);

        char* klient_id = strdup(obszar);
        char* msg;
        sprintf(msg, "Serwer: wiadomosc od klienta %s", klient_id);
        printf("%s\n", msg);
        printf("Wysylam wiadomosc...\n");
        sprintf(msg, "Hej %s tu Serwer %d\n", klient_id, (int)getpid());
        strcpy(obszar, msg);
        sleep(5);

        sem_post(s2);
        printf("Wiadomosc wyslana.\n");
    }
}