#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<time.h>

void handler(int sig)
{
    printf("child of process %d terminated\n", getpid());
}

int main()
{
    signal(SIGCHLD, handler);
    printf("parent process %d\n", getpid());
    int pid = fork();

    if (pid == 0)
    {
        //signal(SIGCHLD, handler);
        printf("child process %d\n", getpid());
        int cid = fork();
        if (cid == 0)
        {
            printf("child child process %d\n", getpid());
            int counter = 0;
            while (counter < 15)
            {
                printf("%d\n", counter);
                counter++;
                sleep(1);
            }
            printf("end of child child process\n");
        }
        else
        {
            sleep(20);
            printf("end of child process\n");
        }
    }
    
        sleep(30);

        int gg = fork();
        if (gg == 0)
        {
            printf("another child %d\n", getpid());
            sleep(5);
            printf("end of another child\n");
        }
        else
        {
            sleep(40);
            printf("end of parent process\n");
        }
    
}