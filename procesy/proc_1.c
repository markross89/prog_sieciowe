#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>

/* Program tworzy proces potomny, każdy proces wypisuje krótką informację o sobie i swojej
rodzinie. */

int main() {
    int pid, ppid;
    printf("Przed wywolaniem funkcji fork.");
    ppid = getpid();
    fflush(stdout);    
    
    if ((pid = fork()) == -1) {
        perror("Wywolanie funkcji fork nie jest mozliwe.");
        exit(1);
    } else {
        if (pid == 0) {
            printf("\nDziecko %d rodzica %d wypisyje jakis tekst.\n", getpid(), ppid);
        } else {
            printf("\nRodzic %d wypisuje jakis tekst, dziecko %d.\n", ppid, pid);
        }
    }

    return 0;
}
