#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<stdlib.h>
/* Program tworzy proces potomny, potem tworzy proces potomny procesu potomnego. Proces
macierzysty zmiania standardową funkcję obsługi sygnału wywoływanego przy zakończeniu
procesu potomnego. Proszę zwrócić uwagę na fakt, że niestandardowa funkcja obsługi sygnału
jest ważna tylko do pierwszego jej użycia, potem jest używana ponownie standardowa funkcja
obsługi sygnału. Aby ciągle była używana niestandardowa funcja obsługi sygnału, należy w
niestandardowej funkcji obsługi sygnału rejestrować ją jako funkcję obsługi sygnału dla tego
konkretnego sygnału.
*/
void dziecko(int i) {
    printf("Dostalem sygnal, numer procesu %d.\n", getpid());
    wait(NULL);
    signal(SIGCHLD, dziecko);
}
int main(int argc, char* argv[]) {
    int pid = 0, ppid;
    ppid = getpid();
    printf("Proces macierzysty ma numer %d a jego rodzic ma numer %d.\n", ppid, getppid());
    signal(SIGCLD, dziecko);
    if ((pid = fork()) == -1) {
        perror("Pomyslne zakonczenie funkcji fork nie jest mozliwe.\n");
        exit(1);
    }
    else {
        if (pid != 0) {
            sleep(10);
        }
        else
        {
            printf("Dziecko ma numer %d a rodzic %d\n", getpid(), getppid());
            sleep(2);
            pid = fork();
            if (pid != 0) {
                sleep(2);
            }
            else {
                printf("Dziecko dziecka ma numer %d a jego rodzic ma numer %d\n", getpid(), getppid());
            }
        }
    }
}