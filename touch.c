#include "imports.h"


int main(int argc, char *argv[])
{

    struct stat st;
    int index = 1;
    while (index < argc)
    {
        if (stat(argv[index], &st) != -1)
        {

            write(1, argv[index], str_len(argv[index]));
            char *buffer = " already exists\n";
            write(1, buffer, str_len(buffer));
            return 0;
        }

        int dest_fd = open(argv[index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dest_fd < 0)
        {

            perror(argv[index]);

            return 0;
        }
        write(1, argv[index], strlen(argv[index]));
        write(1, " created successfully\n", 23);
        index++;
    }
    return 0;
}