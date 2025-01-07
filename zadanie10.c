#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<aio.h>

int async_operation(int* fd, char* buff, int size, int option) {
    struct aiocb* aio = malloc(sizeof(struct aiocb));
    if (aio == NULL) {
        perror("aio_read - malloc");
        return -1;
    }
    aio->aio_fildes = *fd;
    aio->aio_buf = buff;
    aio->aio_nbytes = size;

    if (option == 'w') {
        if (aio_write(aio) == -1) {
            perror("aio - aio_write()");
            return -1;
        }
    }
    else if (option == 'r') {
        if (aio_read(aio) == -1) {
            perror("aio - aio_read()");
            return -1;
        }
    }
    else {
        printf("Zla opcja. 'r' - read ; 'w' - write");
        return -1;
    }
    if (aio_suspend((const struct aiocb* const*)&aio, 1, NULL) == -1) {
        perror("aio - aio_suspend");
        return -1;
    }
    if ((size = aio_return(aio)) == -1) {
        perror("readFile/writeFile - aio_return");
        return -1;
    }
    free(aio);

    return size;
}



int main(int argc, char** argv)
{
    if (argc != 5) {
        printf("Zla liczba argumentow");
        return -1;
    }

    char* mode = argv[4];
    int size = strtol(argv[3], NULL, 0);
    int fd1, fd2, newSize, flags;
    void* buff = malloc(size);

    if (strcmp(mode, "ab") == 0) {
        printf("async blocking mode\n");
        int newSize;

        fd1 = open(argv[1], O_RDONLY);
        fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd1 == -1 || fd2 == -1) {
            perror("fd1 or fd2 - open()");
            return -1;
        }

        newSize = async_operation(&fd1, buff, size, 'r');
        newSize = async_operation(&fd2, buff, newSize, 'w');
        printf("Przekopiowano %d\n", newSize);

        close(fd1);
        close(fd2);
        free(buff);

        return 0;
    }
    else if (strcmp(mode, "sn") == 0) {
        printf("sync non-blocking mode\n");
        flags = O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK;
    }
    else if (strcmp(mode, "sb") == 0) {
        printf("sync blocking mode\n");
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    }
    else {
        printf("Opcja niedostepna! Spróbuj: 'sb', 'sn', 'ab'\n");
        return -1;
    }

    fd1 = open(argv[1], O_RDONLY);
    fd2 = open(argv[2], flags, 0644);
    if (fd1 == -1 || fd2 == -1) {
        perror("fd1 or fd2 - open()");
        return -1;
    }

    if ((newSize = read(fd1, buff, size)) == -1) {
        perror("fd1 - read()");
    }
    if ((newSize = write(fd2, buff, newSize)) == -1) {
        perror("fd2 - write()");
    }
    printf("Przekopiowano %d\n", newSize);

    close(fd1);
    close(fd2);
    free(buff);

    return 0;
}