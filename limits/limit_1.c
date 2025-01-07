/*
Pobranie i ustawienie wielkosci pliku typu core. Polecenia getrlimit
i setrlimit sluza do pobrania i ustawiania roznych limitow w systemie
operacyjnym.
*/

#include<stdio.h>
#include<sys/resource.h>

int main() {
    // declare variable of type struct rlimit:

    // struct rlimit {
    //     rlim_t rlim_cur;  /* Soft limit */
    //     rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
    // };
    struct rlimit pomoc;


    // get data (rlim_cur and rlim_max) from RLIMIT_CORE and assigned it to pomoc struct
    if (getrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas pobierania danych getrlimit");
        return -1;
    }
    printf("soft: %lu, hard: %lu.\n", pomoc.rlim_cur, pomoc.rlim_max);
    pomoc.rlim_cur = 2000;
    // save new rlim_cur value into RLIMIT_CORE file
    if (setrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas zapisywania danych setrlimit");
        return -1;
    }
    // get data (rlim_cur and rlim_max) from RLIMIT_CORE and assigned it to pomoc struct again (with new rlim_cur value)
    if (getrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas pobierania danych getrlimit");
        return -1;
    }    
    printf("soft: %lu, hard: %lu.\n", pomoc.rlim_cur, pomoc.rlim_max);
    return 0;
}