/*
Program wypisuje podstawowe informcje na temat pliku w systmie.
*/

#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>
#include<pwd.h>
#include<stdio.h>

int main(int argc, char* argv[]) {
    int plik;

    // unix struct used to display file system status
    // struct stat {
    //     dev_t st_dev;
    //     ino_t st_ino;
    //     mode_t st_mode;
    //     nlink_t st_nlink;
    //     uid_t st_uid;
    //     gid_t st_gid;
    //     dev_t st_rdev;
    //     off_t st_size;
    //     time_t st_atime;
    //     time_t st_mtime;
    //     time_t st_ctime;
    //     blksize_t st_blksize;
    //     blkcnt_t st_blocks;
    //     mode_t st_attr;
    // };
    struct stat opis;

    // unix time struct to brak down time value
    // struct tm {
    //     int tm_sec;    /* Seconds          [0, 60] */
    //     int tm_min;    /* Minutes          [0, 59] */
    //     int tm_hour;   /* Hour             [0, 23] */
    //     int tm_mday;   /* Day of the month [1, 31] */
    //     int tm_mon;    /* Month            [0, 11]  (January = 0) */
    //     int tm_year;   /* Year minus 1900 */
    //     int tm_wday;   /* Day of the week  [0, 6]   (Sunday = 0) */
    //     int tm_yday;   /* Day of the year  [0, 365] (Jan/01 = 0) */
    //     int tm_isdst;  /* Daylight savings flag */

    //     long tm_gmtoff; /* Seconds East of UTC */
    //     const char* tm_zone;   /* Timezone abbreviation */
    // };
    struct tm* czas;

    if (argc != 2) {
        perror("Bledna ilosc argumentow.\n");
        return -1;
    }
    plik = open(argv[1], O_RDONLY);
    if (plik == -1) {
        perror("Blad otwarcia pliku.\n");
        return -1;
    }

    // fstat(fileDescriptor, &structStat) - return info about file and fill struct with data
    if (fstat(plik, &opis) == -1) {
        perror("Blad funkcji fstat.\n");
        return -1;
    }
    // getpwuid(userUid) - function returns a pointer to a structure containing the broken-out fields of the record in the
    // password database that matches the user ID uid.
    printf("Wlasciciel: %s\n", getpwuid(opis.st_uid)->pw_gecos);
    printf("Rozmiar: %ld\n", opis.st_size);
    // localtime(epoch) - will get time in sec from epoch and fill time struct with data
    czas = localtime(&(opis.st_mtime));
    printf("Czas: %d %d %d %d:%d\n", czas->tm_year, czas->tm_mon, czas->tm_mday, czas->tm_hour, czas->tm_min);
}