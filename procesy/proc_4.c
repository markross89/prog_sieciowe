#include<stdio.h>

/* Program tworzy nowy proces i potok do niego oraz uruchamia program factor i przekauje
parametry wywołania programu jako dane dla niego. Prykladowe wywolanie: a.out 13 15 18. */

int main(int argc, char* argv[]) {
    FILE* f;
    int i;
    char polecenie[1024];
    polecenie[0] = '\0';
    if (f = popen("factor", "w")) {
        for (i = 1; i < argc; i++){
            sprintf(polecenie, "%s %s", polecenie, argv[i]);
        }
        fprintf(f, "%s", polecenie);
        pclose(f);

        return 0;
    }
    else {
        perror("Uruchomienie programu nie jest mozliwe");
        return -1;
    }
}

// char name[1024] - string / array of chars of max size 1024
// fprintf(fileName, "%s", string) - "file print format" save string (char array) to file with specified format
// vfprintf(fileName, "%s", array) - same as fprintf but takes variable arguments.
// sprintf(stream/string, "%s %s", stream/string, string) "stream print format" used in a loop concatenate strings into one stream/string
// popen("functionName", "w/r") - "pipe stream open" - unidirectional connection beetween proceses (not files) (current proces and functionName process) - if
                                            //  functionName process return output we use "r" read as file descriptor and "w" read if functionName
                                            //  read any data. It returns a file wich works as a stream

// pclose(fileName) - "pipe stream close" - closes the popen