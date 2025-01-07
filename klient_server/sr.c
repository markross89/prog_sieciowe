#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#define SIZE 500
#define PATH "fifos/kolejka"

typedef struct message1
{
    long mytype;
    pid_t pid;
} struct1;
struct1* mess1;

typedef struct message2
{
    long mytype;
    char text[SIZE];
} struct2;
struct2* mess2;

int fifo1;
int fifo2;
void sig_handler(int i)
{
    if (i == SIGINT || i == SIGTERM)
    {
        free(mess1);
        free(mess2);
        msgctl(fifo1, IPC_RMID, NULL);
        unlink(PATH);
        exit(0);
    }
}

int main()
{
    key_t key;
    int fd;
    int bytes;

    sigignore(SIGCHLD);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    unlink(PATH);
    fd = open(PATH, O_CREAT | O_EXCL, 0666);
    close(fd);

    key = ftok(PATH, 0);
    fifo1 = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);

    while (1)
    {
        mess1 = (struct1*)malloc(sizeof(struct1));
        const size_t wielkosc1 = sizeof(struct2) - sizeof(long);

        bytes = msgrcv(fifo1, mess1, wielkosc1, 1, 0);

        printf("odczytano %d bajtow od klienta o id %d\n", bytes, (int)mess1->pid);

        int pid;
        pid = fork();
        if (pid == 0)
        {
            key_t key;
            int fifo2;
            int bytes;

            mess2 = (struct2*)malloc(sizeof(struct2));
            size_t wielkosc2 = sizeof(struct2) - sizeof(long);
            key = ftok(PATH, mess1->pid);
            fifo2 = msgget(key, 0666);
            bytes = msgrcv(fifo2, mess2, wielkosc2, 1, 0);
            printf("wiadomosc od klienta : %s", mess2->text);

            mess2->mytype = 3;
            strcpy(mess2->text, "serwer odebral wiadomosc\n");
            wielkosc2 = sizeof(struct2) - sizeof(long);

            msgsnd(fifo2, mess2, wielkosc2, 0);

            free(mess1);
            free(mess2);
        }
    }
    kill(getpid(), SIGTERM);
}


