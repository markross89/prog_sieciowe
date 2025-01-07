#include<stdio.h>

int main()
{
    char* name;
    int nr = 5;
    sprintf(name, "hi %d\n", nr);
    printf("%s", name);
    return 0;
}