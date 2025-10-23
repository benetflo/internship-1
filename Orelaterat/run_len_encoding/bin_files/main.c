

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


int main()
{
    //check if output dir exists otherwise create it
    check_dir();
}