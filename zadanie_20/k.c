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
char* pamiec, * pamiec_dziecko;
char* wiadomosc;
bufor_semaforow ustawienia_semaforow;
bufor_semaforow ustawienia_semaforow_dziecko;
int semafory, semafory_dziecko;

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
        shmdt(pamiec_dziecko);
        printf("Pamiec wyczyszczona!\n");
        exit(-1);
    }
}

void ustaw_semafor_dziecko(int nr, enum operacja op, int nr_wywolania)
{
    ustawienia_semaforow_dziecko.sem_num = nr;
    ustawienia_semaforow_dziecko.sem_op = op;
    ustawienia_semaforow_dziecko.sem_flg = 0;

    if ((semop(semafory_dziecko, &ustawienia_semaforow_dziecko, 1)) == -1)
    {
        printf("Klient: dziecko: semop()[%d] - %s\n", nr_wywolania, strerror(errno));
        free(wiadomosc);
        shmdt(pamiec);
        shmdt(pamiec_dziecko);
        printf("Pamiec wyczyszczona!\n");
        exit(-1);
    }
}


int main()
{
    int id_pamiec, id_pamiec_dziecko;
    int klucz_semafory, klucz_pamiec, klucz_semafory_dziecko, klucz_pamiec_dziecko;

    if ((klucz_pamiec = ftok(sciezka, 0)) == -1 || (klucz_semafory = ftok(sciezka, 1)) == -1)
    {
        perror("klient: ftok()");
        return -1;
    }

    if ((semafory = semget(klucz_semafory, 3, 0600)) == -1)
    {
        perror("Klient: semget()[1]");
        return -1;
    }

    if ((id_pamiec = shmget(klucz_pamiec, 50 * sizeof(char), 0600)) == -1)
    {
        perror("Klient: shmget()[1]");
        return -1;
    }

    pamiec = shmat(id_pamiec, (char*)0, 0);

    wiadomosc = (char*)malloc(100 * sizeof(char));

    printf("Klient nr %d:\n", (int)getpid());

    ustaw_semafor(1, CZEKAJ, 1);

    sprintf(wiadomosc, "%d", (int)getpid());
    strcpy(pamiec, wiadomosc);

    ustaw_semafor(0, ZWOLNIJ, 2);
    ustaw_semafor(2, CZEKAJ, 3);
    if ((klucz_pamiec_dziecko = ftok(sciezka, getpid())) == -1 || (klucz_semafory_dziecko = ftok(sciezka, getpid() + 1)) == -1)
    {
        perror("klient: dziecko: ftok()");
        return -1;
    }
    if ((semafory_dziecko = semget(klucz_semafory_dziecko, 2, 0600)) == -1)
    {
        perror("Klient: dziecko[1] semget()");
        return -1;
    }
    if ((id_pamiec_dziecko = shmget(klucz_pamiec_dziecko, 50 * sizeof(char), 0600)) == -1)
    {
        perror("Klient: dziecko[1] shmget()[1]");
        return -1;
    }

    pamiec_dziecko = shmat(id_pamiec_dziecko, (char*)0, 0);

    ustaw_semafor_dziecko(0, CZEKAJ, 1);

    wiadomosc = strdup(pamiec_dziecko);
    printf("%s", wiadomosc);

    scanf("%s", wiadomosc);
    strcpy(pamiec_dziecko, wiadomosc);

    ustaw_semafor_dziecko(1, ZWOLNIJ, 1);
    ustaw_semafor_dziecko(2, CZEKAJ, 2);

    wiadomosc = strdup(pamiec_dziecko);
    printf("Serwer -> Klient: %s\n", wiadomosc);

    printf("Koniec pracy klienta\n");
    free(wiadomosc);
    shmdt(pamiec);
    shmdt(pamiec_dziecko);
    ustaw_semafor_dziecko(3, ZWOLNIJ, 3);

    return 0;
}