#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main()
{
   
    int fd[2];
    if (pipe(fd) == -1)
        printf("pipe error");
    int pid;
    if ((pid = fork()) == -1)
        printf("fork error");
    if(pid == 0)
    {
        close(fd[0]);
        int a;
        printf("podaj wartosc zmiennej w procesie %d: \n", getpid());
        scanf("%d", &a);
        write(fd[1], &a, sizeof(int));
        close(fd[1]);
    }
    else
    {
        close(fd[1]);
        int b;
        read(fd[0], &b, sizeof(int));
        printf("zmienna odebrana w procesie %d ma wartosc %d\n", getpid(), b);
        close(fd[0]);
    }
    return 0;
}