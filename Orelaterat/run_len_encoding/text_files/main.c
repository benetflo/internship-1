#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void check_dir()
{
    struct stat st = {0};

    if(stat("output", &st) == -1)
    {
        mkdir("output", 0700);
    }
}

#define RD_ONLY     "r"
#define WR_ONLY     "w"

const char* old_file_name = "text.txt";
const char* new_file_name = "output/compressed_text.txt";

int main()
{
    //check if output dir exists otherwise create it
    check_dir();

    FILE* pTextFile = fopen(old_file_name, RD_ONLY);
    FILE* pNewTextFile = fopen(new_file_name, WR_ONLY);

    if(pTextFile == NULL|| pNewTextFile == NULL)
    {
        printf("Error %d, from fopen: %s\n", errno, strerror(errno));
        exit(1);
    }

    int c = getc(pTextFile);
    uint8_t counter = 1;
    int curr_c = c;
    int index = 0;

    while(c != EOF)
    {
        if(curr_c == c)
        {
            counter++;
        }else
        {
            fprintf(pNewTextFile, "%c:%d;", curr_c, counter);
            counter = 1;
            curr_c = c; 
        }
        c = getc(pTextFile);
        
        // add last element when EOF is reached
        if(c == EOF)
        {
            fprintf(pNewTextFile, "%c:%d;", curr_c, counter);
        }
    }

    fclose(pTextFile);
    fclose(pNewTextFile);

    return 0;
}