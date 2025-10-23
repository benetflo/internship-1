#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#define RD_BIN_ONLY                         "rb"
#define WR_BIN_ONLY                         "wb"

#define ARRAY_SIZE(arr)                     (sizeof(arr) / sizeof((arr)[0]))
#define BUFFER_SIZE                         1024

const char* old_file_path = "test_rle.bin";
const char* new_file_path = "output/compressed.bin";

void check_dir()
{
    struct stat st = {0};

    if(stat("output", &st) == -1)
    {
        mkdir("output", 0700);
    }
}


int main()
{
    //check if output dir exists otherwise create it
    check_dir();
    
    
    FILE* pMP3 = fopen(old_file_path, RD_BIN_ONLY);
    FILE* pNewMP3 = fopen(new_file_path, WR_BIN_ONLY);
    
    if(pMP3 == NULL)
    {
        printf("Error %d, from fopen: %s\n", errno, strerror(errno));
        exit(1);
    }
    
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;
    int first_byte = 1;
    unsigned char curr_byte = 0;
    uint8_t counter = 0;

    while((bytes_read = fread(buffer, 1, BUFFER_SIZE, pMP3)) > 0)
    {
        
        for(size_t i = 0; i < bytes_read; i++)
        { 
            unsigned char b = buffer[i];
            if(first_byte)
            {
                curr_byte = b;
                counter = 1;
                first_byte = 0;
            }
            else if(b == curr_byte)
            {
                counter++;
                if(counter == 255)
                {
                    fwrite(&curr_byte, 1, 1, pNewMP3);
                    fwrite(&counter, 1, 1, pNewMP3);
                    counter = 0;
                }
            }else
            {
                if(counter > 0)
                {
                    fwrite(&curr_byte, 1, 1, pNewMP3);
                    fwrite(&counter, 1, 1, pNewMP3);
                }
                curr_byte = b;
                counter = 1;
            }
        }    
    }
    if(counter > 0)
    {
        fwrite(&curr_byte, 1, 1, pNewMP3);
        fwrite(&counter, 1, 1, pNewMP3);
    }

    fclose(pMP3);
    fclose(pNewMP3);

    return 0;
}