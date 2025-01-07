/*
Przyklad programu zapisujacego dane do pliku w sposob asynchroniczny, w trybie blokowanym.
Program poprawnie dziala na maszynie sxterm, na maszynie ultra60
asynchroniczna obsluga operacji wejscia i wyjscia nie jest obslugiwana i otrzymujemy stosowny komunikat.
*/

#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<aio.h>
#include<errno.h>
#include<stdint.h>

int main(int argc, char* argv[]) {
    int liczbaIteracji = 1, i, result;
    char buf[1024];
    struct aiocb *message;
    size_t size = 0;


    // asynchronous I / O control block struct
    //     struct aiocb {
    //     int             aio_fildes;     /* File descriptor */
    //     off_t           aio_offset;     /* File offset */
    //     volatile void*  aio_buf;        /* Location of buffer */
    //     size_t          aio_nbytes;     /* Length of transfer */
    //     int             aio_reqprio;    /* Request priority offset */
    //     struct sigevent aio_sigevent;   /* Signal number and value */
    //     int             aio_lio_opcode; /* Operation to be performed */
    //                  
    //     int      aio_cancel(int, struct aiocb*);
    //     int      aio_error(const struct aiocb*);
    //     int      aio_fsync(int, struct aiocb*);
    //     int      aio_read(struct aiocb*);
    //     ssize_t  aio_return(struct aiocb*);
    //     int      aio_suspend(const struct aiocb* const [], int, const struct timespec*);
    //     int      aio_write(struct aiocb*);
    //     int      lio_listio(int, struct aiocb* restrict const [restrict], int, struct sigevent* restrict);
    // };
    // struct aiocb* message;
    // size_t size = 0;

    /* zaalokowanie struktury zawierajacej parametry dla komunikacji asynchronicznej */
    message = malloc(sizeof(struct aiocb));
    printf("Program testowy.\n");
    if (argc > 1) {

        /* konwersja pierwszego parametru wywolania na tym int */
        liczbaIteracji = strtol(argv[1], NULL, 10);
    }

    /* otwarcie pliku do pisania w trybie blokowanym z flaga dopisywania do pliku i zerowania pliku przy otwarciu,
       tego typu zabiegi sa konieczne dla zapisu asynchronicznego gdyz brak flagi O_APPEND powoduje, ze kazde
       asynchroniczne wywolanie tworzy nowy pusty plik, flaga O_TRUNC zapewnia, ze kazde uruchomienie programu zeruje plik  */
    int plik = open("testowy.txt", O_CREAT | O_APPEND | O_TRUNC | O_WRONLY, 0644);

    for (i = 0; i < liczbaIteracji; i++) {

        /* tworzenie wiadomosci do zapisania w pliku */
        sprintf(buf, "Zawartosc testowa, interacja numer %d.\n", i);

        /* konfiguracja struktury zawierajacej parametry dla komunikacji asynchronicznej */
        message->aio_fildes = plik;
        message->aio_buf = buf;
        message->aio_nbytes = strlen(buf);

        /* zapisanie wiadomosci do pliku w sposob asynchroniczny */
        result = aio_write(message);
        if (result) {
            printf("Blad operacji zapisu aio_write: %s.\n", strerror(errno));
            return -1;
        }

        // zamiast uzyc aio_suspend mozemy dac programowi inne zadanie do wykonania w czasie gdy dane sa zapisywane do pliku
        // np drukowanie inkrementowanej liczby

        // int counter = 0;
        // while (aio_error(message) == EINPROGRESS)
        //         printf("%lu\n", counter++);

                
        /* czekanie na zakonczenie asynchronicznego zapisu - inaczej result bedzie pusty */
        result = aio_suspend((const struct aiocb* const*)&message, 1, NULL);
        if (result) {
            printf("Blad operacji zapisu aio_write: %s.\n", strerror(errno));
            return -1;
        }

        /* pobranie liczby bajtow zapisanych do pliku */
        size += aio_return(message);
    }

    /* komunikat z podsumowaniem liczby zapisanych bajtow do pliku */
    printf("Zapisalem do pliku %lu bajtow.\n", size);

    /* zamkniecie pliku */
    close(plik);
    return 0;
}

// ???? do we not suppose to free message struct memory if something goes wrong and at the end of program?