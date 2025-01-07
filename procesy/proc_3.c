#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>

/* Program tworzy proces potomny i uruchamia zadane polecenie w ramach procesu potomnego.
Przykladowe wywolanie programu: a.out cal 10 2024. */

int main(int argc, char** argv) {
    char** polecenie;
    long i = 0;
    int pid;
    polecenie = (char**)malloc(sizeof(char*) * argc);
    while (i < argc - 1) {
        polecenie[i] = argv[i + 1];
        i++;
    }
    polecenie[i] = NULL;
    if ((pid = fork()) == -1) {
        perror("Pomyslne zakonczenie funkcji fork nie jest mozliwe.\n");
        exit(1);
    }
    else {
        if (pid == 0) {
            execvp(polecenie[0], polecenie);
        }
        else {
            waitpid((pid_t)-1, NULL, 0);
            printf("\nPolecenie wraz z argumentami zostalo wykonane.\n");
        }
    }
    free(polecenie);
}