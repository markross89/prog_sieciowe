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
bufor_semaforow ustawienia_semaforow_dziecko;
int semafory, id_pamiec, semafory_dziecko, id_pamiec_dziecko;
char* sciezka = "./sem_serwer";
char* pamiec, * pamiec_dziecko;

void wyczysc(int s)
{
    printf("\nKoniec pracy serwera!\n");
    shmdt(pamiec);
    shmctl(id_pamiec, IPC_RMID, NULL);
    semctl(semafory, 0, IPC_RMID, NULL);
    unlink(sciezka);
    printf("Pamiec wyczyszczona!\n");
    exit(0);
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

void ustaw_semafor_dziecko(int nr, enum operacja op, int nr_wywolania)
{
    ustawienia_semaforow_dziecko.sem_num = nr;
    ustawienia_semaforow_dziecko.sem_op = op;
    ustawienia_semaforow_dziecko.sem_flg = 0;

    if ((semop(semafory_dziecko, &ustawienia_semaforow_dziecko, 1)) == -1)
    {
        printf("Serwer: dziecko: funkcja semop()[%d] - %s\n", nr_wywolania, strerror(errno));
        kill(getpid(), SIGINT);
    }
}

int main()
{
    int klucz_semafory, klucz_pamiec, plik = 0;

    signal(SIGTERM, wyczysc);
    signal(SIGINT, wyczysc);
    sigignore(SIGCHLD);

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

    if ((semafory = semget(klucz_semafory, 3, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
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

    unsigned short wartosci_poczatkowe_semaforow[] = { 0, 1 ,0 };

    if ((semctl(semafory, 0, SETALL, wartosci_poczatkowe_semaforow)) == -1)
    {
        perror("Serwer: semctl[1]");
        shmdt(pamiec);
        shmctl(id_pamiec, IPC_RMID, NULL);
        semctl(semafory, 0, IPC_RMID, NULL);
        unlink(sciezka);
        return -1;
    }

    printf("Serwer:\n\n");
    for (;;)
    {
        ustaw_semafor(0, CZEKAJ, 1);

        char* klient_id_ptr = strdup(pamiec);
        char* end;
        int klient_id = strtol(klient_id_ptr, &end, 0);
        printf("Serwer: Klient: %d\n", klient_id);

        ustaw_semafor(1, ZWOLNIJ, 2);

        int pid;
        if ((pid = fork()) == -1)
        {
            perror("Serwer: fork()");
            wyczysc(1);
            return -1;
        }

        if (pid == 0)
        {
            int klucz_semafory_dziecko, klucz_pamiec_dziecko;
            if ((klucz_pamiec_dziecko = ftok(sciezka, klient_id)) == -1 || (klucz_semafory_dziecko = ftok(sciezka, klient_id + 1)) == -1)
            {
                perror("Serwer: dziecko: ftok()");
                return -1;
            }

            if ((semafory_dziecko = semget(klucz_semafory_dziecko, 4, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
            {
                perror("Serwer: dziecko: semget()");
                return -1;
            }

            if ((id_pamiec_dziecko = shmget(klucz_pamiec_dziecko, 50 * sizeof(char), 0600 | IPC_CREAT | IPC_EXCL)) == -1)
            {
                perror("Serwer: dziecko: shmget()");
                semctl(semafory_dziecko, 0, IPC_RMID, NULL);
                return -1;
            }

            pamiec_dziecko = shmat(id_pamiec_dziecko, (char*)0, 0);

            unsigned short wartosci_poczatkowe_semaforow_dziecko[] = { 0, 0, 0, 0 };

            if ((semctl(semafory_dziecko, 0, SETALL, wartosci_poczatkowe_semaforow_dziecko)) == -1)
            {
                perror("Serwer: dziecko: semctl[1]");
                shmdt(pamiec_dziecko);
                shmctl(id_pamiec_dziecko, IPC_RMID, NULL);
                semctl(semafory_dziecko, 0, IPC_RMID, NULL);
                return -1;
            }
            ustaw_semafor(2, ZWOLNIJ, 3);

            char* wiadomosc = "Serwer: Podaj swoje imie...\n";
            strcpy(pamiec_dziecko, wiadomosc);

            ustaw_semafor_dziecko(0, ZWOLNIJ, 2);
            ustaw_semafor_dziecko(1, CZEKAJ, 3);

            wiadomosc = strdup(pamiec_dziecko);
            char* c = wiadomosc;
            while (*wiadomosc) {
                *wiadomosc = toupper((unsigned char)*wiadomosc);
                wiadomosc++;
            }
            strcpy(pamiec_dziecko, c);
            printf("Serwer: Zmienilem twoje imie na drukowane litery\n");

            ustaw_semafor_dziecko(2, ZWOLNIJ, 4);
            ustaw_semafor_dziecko(3, CZEKAJ, 4);

            shmdt(pamiec_dziecko);
            shmctl(id_pamiec_dziecko, IPC_RMID, NULL);
            semctl(semafory_dziecko, 0, IPC_RMID, NULL);

            return 0;
        }
    }
}

