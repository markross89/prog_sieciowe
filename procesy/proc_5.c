/*
Wspolpraca z kalkulatorem bc z wykorzystaniem potoków. Przykładowe wywolanie programu a.out 4+7 lub a.out "5 +
7". Aby wyliczyc wartosc funkcji sinus mozna uzyc polecenia a.out "s(7)".
*/

#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(int argc, char* argv[]) {
  
    int pid, fd1[2], fd2[2];
    if (argc != 2) {
        perror("Bledna liczba argumentow.\n");
        return -1;
    }
    else {
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
                    // closing unused streams
                    close(fd2[0]);  
                    close(fd1[1]);
                    // create char array that holds comand and its params 
                    char** pomoc = malloc(3 * sizeof(char*));
                    pomoc[0] = strdup("bc");
                    pomoc[1] = strdup("-l");
                    pomoc[2] = NULL;
                    // change input and output from stdin and stdout to fd1 and fd2 (pipe) 0 - stdin , 1 - stdout
                    dup2(fd1[0], 0); //read
                    dup2(fd2[1], 1); //write
                    // this child process is replaced now by bc -a (it is waiting for the input and will get it from fd1 read stream) 
                    // and then it will output to fd2 the result
                    execvp(pomoc[0], pomoc);
                }
                else {
                    // closing unused streams
                    close(fd1[0]);
                    close(fd2[1]);
                    // create a file pointers
                    FILE* f1, * f2;
                    // create a buffor
                    char* bufor;
                    // open file descriptors as files
                    f1 = fdopen(fd1[1], "w"); //write
                    f2 = fdopen(fd2[0], "r"); //read
                    // write function parameter to f1 wich is assigned to fd1 write stream (it will pipe this value to child process)
                    fprintf(f1, "%s\nquit\n", argv[1]);
          //???     // don't know why but i think we flush it to send the value imidietly and empty fprintf buffer
                    fflush(f1);
                    // allocate memory for buffor 
                    bufor = (char*)malloc(1000 * sizeof(char));
                    // read from f2 wich is assigned to fd2 read stream the result of child bc -l function outputed in child process
                    fgets(bufor, 1000, f2);
                    // prints the result
                    printf("Wynik %s", bufor);
                    // empty buffor
                    free(bufor);
                }
            }
            return 0;
        }
    }
}

//  ????? do we not suppose to close files in parent and file descriptors in child processes