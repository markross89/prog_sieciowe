#include<sys/sem.h>
#include<sys/shm.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<errno.h>
#include<signal.h>

enum operacja { CZEKAJ = -1, ZWOLNIJ = 1 };

typedef struct sembuf bufor_semaforow;

char* sciezka = "./sem_serwer";
char* pamiec;
char* wiadomosc;
bufor_semaforow ustawienia_semaforow;
int semafory;

void ustaw_semafor(int nr, enum operacja op, int nr_wywolania)
{
    ustawienia_semaforow.sem_num = nr;
    ustawienia_semaforow.sem_op = op;
    ustawienia_semaforow.sem_flg = 0;

    if ((semop(semafory, &ustawienia_semaforow, 1)) == -1)
    {
        printf("Klient: semop()[%d] - %s\n", nr_wywolania, strerror(errno));
        free(wiadomosc);
        shmdt(pamiec);
        printf("Pamiec wyczyszczona!\n");
        exit(-1);
    }
}

int main()
{
    int id_pamiec;
    int klucz_semafory, klucz_pamiec, plik = 0;

    if ((klucz_pamiec = ftok(sciezka, 0)) == -1 || (klucz_semafory = ftok(sciezka, 1)) == -1)
    {
        perror("klient: ftok()");
        return -1;
    }

    if ((semafory = semget(klucz_semafory, 1, 0600)) == -1)
    {
        perror("Klient: semget()");
        return -1;
    }

    if ((id_pamiec = shmget(klucz_pamiec, 50 * sizeof(char), 0600)) == -1)
    {
        perror("Klient: shmget()");
        return -1;
    }

    pamiec = shmat(id_pamiec, (char*)0, 0);

    wiadomosc = (char*)malloc(100 * sizeof(char));

    printf("Klient nr %d:\n", (int)getpid());

    ustaw_semafor(0, CZEKAJ, 1);

    sprintf(wiadomosc, "Moj numer procesu to %d", (int)getpid());
    strcpy(pamiec, wiadomosc);

    ustaw_semafor(1, ZWOLNIJ, 2);
    ustaw_semafor(1, CZEKAJ, 3);

    wiadomosc = strdup(pamiec);
    printf("%s", wiadomosc);
    scanf("%s", wiadomosc);
    strcpy(pamiec, wiadomosc);

    ustaw_semafor(1, ZWOLNIJ, 4);
    ustaw_semafor(1, CZEKAJ, 5);

    wiadomosc = strdup(pamiec);
    printf("Serwer -> Klient: %s\n", wiadomosc);

    ustaw_semafor(0, ZWOLNIJ, 6);

    printf("Koniec pracy klienta\n");
    free(wiadomosc);
    shmdt(pamiec);

    return 0;
}