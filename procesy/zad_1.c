#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<signal.h>
#include<string.h>

static const char* katalog = "/tmp/rafalb-FIFO-TMP-SERWER";
int f, f3;

void ala(int i) {
    close(f);
    close(f3);
    unlink(katalog);
    exit(0);
}

int main() {
    int f1, f2;
    char bufor[1000], * pom = bufor;
    pid_t nr_proc, pid;

    signal(SIGTERM, ala);
    signal(SIGINT, ala);
    sigignore(SIGCHLD);
    unlink(katalog);
    if (mknod(katalog, S_IFIFO | 0600, 0) == -1) {
        perror("Serwer: Blad tworzenia kolejki FIFO. [1]\n");
        return -1;
    }
    else {
        if ((f = open(katalog, O_RDONLY)) == -1 || (f3 = open(katalog, O_WRONLY)) == -1) {
            perror("Serwer: Blad otwarcia kolejki FIFO. [2]\n");
            return -1;
        }
        else {
            for (;;) {
                read(f, &nr_proc, sizeof(pid_t));
                if ((pid = fork()) == -1) {
                    perror("Wywolanie funkcji fork nie powiodlo sie.\n");
                    return -1;
                }
                else {
                    if (pid == 0) {
                        strcpy(pom, "/tmp/rafalb-FIFO-TMP-2-");
                        pom += strlen(pom);
                        sprintf(pom, "%d", (int)nr_proc);
                        if ((f1 = open(bufor, O_RDONLY)) == -1) /* kolejka do czytania */ {
                            perror("Serwer: Blad otwarcia kolejki FIFO. [3]\n");
                            close(f);
                            return -1;
                        }
                        else {
                            *(pom - 2) = '1';
                            if ((f2 = open(bufor, O_WRONLY)) == -1) /* kolejka do pisania */ {
                                perror("Serwer: Blad otwarcia kolejki FIFO. [4]\n");
                                close(f1);
                                close(f);
                                return -1;
                            }
                            else {
                                double a, b;

                                read(f1, &a, sizeof(double));
                                b = a * a;
                                write(f2, &b, sizeof(double));
                            }
                        }
                        close(f1);
                        close(f2);
                        close(f);
                        return 0;
                    }
                    else {
                        sigignore(SIGCHLD);
                    }
                }
            }
        }
    }
}