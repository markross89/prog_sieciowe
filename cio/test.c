#include<stdio.h>
#include<stdlib.h>

int main()
{
    //--------------------------------------------READ FILE-------------------------------------------------------------------
        // FILE* ptr;
        // ptr = fopen("file.txt", "r");
        // if (!ptr) {
        //     printf("file not found");
        //     return 1;
        // }

        // ----line by line----

        // char text[200];
        // while (fgets(text, 200, ptr))
        //     printf("%s", text);
        // printf("\n");



        // ----char by char----

        // char c;
        // while ((c = fgetc(ptr)) != EOF)
        //     printf("%c", c);
        // printf("\n");



        // ----formated line by formated line

        // char text[200];
        // while(fscanf(ptr, "%s", text) == 1)
        //     printf("%s", text);
        // printf("\n");

        // char text[100];
        // char ptr1[100];
        // char ptr2[100];
        // while(fscanf(ptr, "%s %s %s", text, ptr1, ptr2) == 3)
        //     printf("%s %s %s", text, ptr1, ptr2);
        // printf("\n");

    //------------------------------------WRITE TO A FILE------------------------------------------------------

    FILE* ptr;
    ptr = fopen("file.txt", "w");
    if (!ptr) {
        printf("Error");
        return 1;
    }

    char text[] = "siema tu marek";

    //---------fputs - will put any text without formating into file--------
    
    // fputs(text, ptr);



    //----------fprintf - will input formated string into file----------

    // fprintf(ptr, "to mój string: %s\n hahha\n", text);


    fclose(ptr);
    return 0;
}

// ``r''   Open text file for reading.  The stream is positioned at the
//          beginning of the file.

//  ``r+''  Open for reading and writing.  The stream is positioned at the
//          beginning of the file.

//  ``w''   Truncate file to zero length or create text file for writing.
//          The stream is positioned at the beginning of the file.

//  ``w+''  Open for reading and writing.  The file is created if it does not
//          exist, otherwise it is truncated.  The stream is positioned at
//          the beginning of the file.

//  ``a''   Open for writing.  The file is created if it does not exist.  The
//          stream is positioned at the end of the file.  Subsequent writes
//          to the file will always end up at the then current end of file,
//          irrespective of any intervening fseek(3) or similar.

//  ``a+''  Open for reading and writing.  The file is created if it does not
//          exist.  The stream is positioned at the end of the file.  Subse-
//          quent writes to the file will always end up at the then current
//          end of file, irrespective of any intervening fseek(3) or similar.

//  ``rewind''  usefull with 'a+' specifier when we first append text to file it will point to end of it waiting to append,
//              but to read it from the begining we need to point to begining with rewind(fileName)