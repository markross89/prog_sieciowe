#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(int argc, char* argv[]) {
    int pid, fd1[2], fd2[2];


    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("Otwarcie laczy nie jest mozliwe.\n");
        return -1;
    }
    else {
        if ((pid = fork()) == -1) {
            perror("Pomyslne zakonczenie funkcji fork nie jest mozliwe.\n");
            return -1;
        }
        else {
            if (pid == 0) {
                char** pomoc = malloc(3 * sizeof(char*));
                pomoc[0] = strdup("bc");
                pomoc[1] = strdup("-l");
                pomoc[2] = NULL;
                close(fd2[0]);
                close(fd1[1]);
                dup2(fd1[0], 0);
                dup2(fd2[1], 1);
                execvp(pomoc[0], pomoc);
            }
            else {
                FILE* f1, * f2;
                char* bufor;
                int l = 0;
                close(fd1[0]);
                close(fd2[1]);
                f1 = fdopen(fd1[1], "w");
                f2 = fdopen(fd2[0], "r");
                bufor = (char*)malloc(1000 * sizeof(char));
                int arg_nr = 1;
                while (arg_nr < argc)
                {
                    fprintf(f1, "%s\n", argv[arg_nr]);
                    fflush(f1);
                    fgets(bufor, 1000, f2);
                    printf("%s = %s", argv[arg_nr], bufor);
                    arg_nr++;
                }
                free(bufor);
            }
        }
        return 0;

    }
}