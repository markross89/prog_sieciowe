#include<stdio.h>
#include<string.h>
#include<unistd.h>
// program wykorzystuje program ed do utworzenia lub modyfikacji przez dodanie tekstu do pliku. 
// przykladowe wywolanie    ./zadanie7 plik.txt   
// zamiast tworzyc file dla odmiany wykorzystuje file descriptor do komunikacji
int main(int argc, char* argv[]) {
    FILE* f;
    int i;
    char buff[1024];
    if (argc != 2) {
        printf("Zla liczba argumentow");
        return -1;
    }
    char komenda[500];
    strcpy(komenda, "ed ");
    strcat(komenda, argv[1]);
    if (f = popen(komenda, "w")) {
        sleep(1);
        printf("Podaj text aby dodać do pliku\n");
        scanf("%s", buff);
        printf("Zawartość po modyfikacji: \n");
        if (fprintf(f, "a\n%s\n.\n,p\nwq\n", buff) < 0) {
            perror("Blad zapisu");
            return -1;
        }
        pclose(f);
        return 0;
    }
    else {
        perror("Uruchomienie programu nie jest mozliwe");
        return -1;
    }
    return 0;
}