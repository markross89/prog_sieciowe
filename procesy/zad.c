#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
// program przesyla polecenie z procesu macierzystego do programu dzialajaceg na miejscu dziecka, a nastepnie odbiera dane w procesie
// macierzystym i drukuje na sdtout
// wywolanie  ./zadanie8 plik.txt

int main(int argc, char* argv[])
{
    int fd1[2];
    int fd2[2];
    int pid;
    char** polecenie = (char**)malloc(1000 * sizeof(char*));
    polecenie[0] = "ed";
    if (argc != 2)
    {
        printf("Wrong numnber of arguments !\n");
        return -1;
    }
    polecenie[1] = argv[1];

    if (pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        perror("Open pipes not possible !\n");
        return -1;
    }
    if ((pid = fork()) == -1)
    {
        perror("Forking failed !\n");
        return -1;
    }
    else
    {
        if (pid == 0)
        {
            close(fd1[0]);
            close(fd2[1]);
            dup2(fd1[1], 1);
            dup2(fd2[0], 0);
            wait(2);
            execvp(polecenie[0], polecenie);
        }
        else
        {
            close(fd1[1]);
            close(fd2[0]);

            char* buff = (char*)malloc(1000 * sizeof(char));

            write(fd2[1], ",p\nwq\n", 1000);

            read(fd1[0], buff, 1000);
            printf("%s", buff);

            close(fd1[0]);
            close(fd2[1]);

            free(buff);
        }
    }

    return 0;
}