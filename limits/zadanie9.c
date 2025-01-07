#include<stdio.h>
#include<sys/resource.h>

int main() {

    struct rlimit pomoc;

    if (getrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas pobierania danych getrlimit");
        return -1;
    }
    printf("soft: %lu, hard: %lu.\n", pomoc.rlim_cur, pomoc.rlim_max);
    pomoc.rlim_cur = 2000;
    if (setrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas zapisywania danych setrlimit");
        return -1;
    }
    if (getrlimit(RLIMIT_CORE, &pomoc) == -1) {
        perror("Wystapil problem podczas pobierania danych getrlimit");
        return -1;
    }    
    printf("soft: %lu, hard: %lu.\n", pomoc.rlim_cur, pomoc.rlim_max);
    return 0;
}