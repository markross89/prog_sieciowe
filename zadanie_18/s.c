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
#include<ctype.h>

enum operacja { CZEKAJ = -1, ZWOLNIJ = 1 };

typedef struct sembuf bufor_semaforow;
bufor_semaforow ustawienia_semaforow;
int semafory, id_pamiec;
char* sciezka = "./sem_serwer";
char* pamiec;

void wyczysc(int s)
{
    if (s == SIGINT || s == SIGTERM)
    {
        printf("\nKoniec pracy serwera!\n");
        shmdt(pamiec);
        shmctl(id_pamiec, IPC_RMID, NULL);
        semctl(semafory, 0, IPC_RMID, NULL);
        unlink(sciezka);
        printf("Pamiec wyczyszczona!\n");
        exit(0);
    }
}

void ustaw_semafor(int nr, enum operacja op, int nr_wywolania)
{
    ustawienia_semaforow.sem_num = nr;
    ustawienia_semaforow.sem_op = op;
    ustawienia_semaforow.sem_flg = 0;

    if ((semop(semafory, &ustawienia_semaforow, 1)) == -1)
    {
        printf("Serwer: funkcja semop()[%d] - %s\n", nr_wywolania, strerror(errno));
        kill(getpid(), SIGINT);
    }
}

int main()
{
    int klucz_semafory, klucz_pamiec, plik = 0;

    signal(SIGTERM, wyczysc);
    signal(SIGINT, wyczysc);

    unlink(sciezka);
    if ((plik = open(sciezka, O_CREAT | O_EXCL, 0600)) == -1)
    {
        perror("Serwer: open()");
        return -1;
    }
    close(plik);

    if ((klucz_pamiec = ftok(sciezka, 0)) == -1 || (klucz_semafory = ftok(sciezka, 1)) == -1)
    {
        perror("Serwer: ftok()");
        unlink(sciezka);
        return -1;
    }

    if ((semafory = semget(klucz_semafory, 2, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        perror("Serwer: semget()");
        unlink(sciezka);
        return -1;
    }

    if ((id_pamiec = shmget(klucz_pamiec, 50 * sizeof(char), 0600 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        perror("Serwer: shmget()");
        semctl(semafory, 0, IPC_RMID, NULL);
        unlink(sciezka);
        return -1;
    }

    pamiec = shmat(id_pamiec, (char*)0, 0);

    unsigned short wartosci_poczatkowe_semaforow[] = { 1, 0};

    if ((semctl(semafory, 0, SETALL, wartosci_poczatkowe_semaforow)) == -1)
    {
        perror("Serwer: semctl[1]");
        shmdt(pamiec);
        semctl(semafory, 0, IPC_RMID, NULL);
        unlink(sciezka);
        return -1;
    }

    printf("Serwer:\n\n");
    for (;;)
    {
        ustaw_semafor(1, CZEKAJ, 1);

        char* wiadomosc = strdup(pamiec);
        printf("Serwer <- Klient: %s\n", wiadomosc);
        wiadomosc = "Serwer -> Klient: Podaj swoje imie\n";
        strcpy(pamiec, wiadomosc);

        ustaw_semafor(1, ZWOLNIJ, 2);
        ustaw_semafor(1, CZEKAJ, 3);

        wiadomosc = strdup(pamiec);
        char* c = wiadomosc;
        while (*wiadomosc) {
            *wiadomosc = toupper((unsigned char)*wiadomosc);
            wiadomosc++;
        }
        strcpy(pamiec, c);
        printf("Zmienilem twoje imie na drukowane litery\n\n");

        ustaw_semafor(1, ZWOLNIJ, 4);
    }
}