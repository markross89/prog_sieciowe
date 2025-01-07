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

typedef struct message3
{
    long mytype;
    char text[SIZE];
} struct3;

struct3* mess3;

int fifo1;
int fifo2;
void sig_handler(int i)
{
    if (i == SIGINT || i == SIGTERM)
    {
        free(mess1);
        free(mess2);
        msgctl(fifo2, IPC_RMID, NULL);
        exit(0);
    }
}

int main()
{
    key_t key;
    int bytes;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    key = ftok(PATH, 0);
    fifo1 = msgget(key, 0666);

    mess1 = (struct1*)malloc(sizeof(struct1));
    mess1->mytype = 1;
    mess1->pid = getpid();
    const size_t wielkosc1 = sizeof(struct2) - sizeof(long);

    msgsnd(fifo1, mess1, wielkosc1, 0);

    printf("wyslano do serwera\n");

    key = ftok(PATH, getpid());
    fifo2 = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);

    mess2 = (struct2*)malloc(sizeof(struct2));
    mess2->mytype = 1;
    printf("Podaj text aby wyslac na server\n");
    fgets(mess2->text, 100, stdin);
    const size_t wielkosc2 = sizeof(struct2) - sizeof(long);

    msgsnd(fifo2, mess2, wielkosc2, 0);

    printf("wyslano do serwera\n");

    msgrcv(fifo2, mess2, wielkosc2, 3, 0);

    printf("%s", mess2->text);

    kill(getpid(), SIGINT);
}