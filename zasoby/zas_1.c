/*
Program wypisuje informacje o efektywnym biezacym uzytkowniku. Program ma problemy z poprawna praca na wszelkich emulatorach systemow typu uniksowego.
*/

#include<unistd.h>
#include<pwd.h>
#include<stdio.h>

int main() {
    // getpwuid(userUid) - function returns a pointer to a structure containing the broken-out fields of the record in the
    // password database that matches the user ID uid.
    printf("Uzytkownik: %s.\n", getpwuid(geteuid())->pw_gecos);
}