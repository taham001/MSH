#include "imports.h"

int main(int argc, char *argv[])
{
    

    if (argc == 1)
    {

        char *buffer = malloc(sizeof(char) * 128);
        if (buffer == NULL)
        {
            perror("Failed");
            return 1;
        }

        int size = 128;

        while (1)
        {

            int bytes = 0;
            int bytes_read = 0;
            while ((bytes = read(0, buffer + bytes_read, 64)) > 0)
            {

                bytes_read += bytes;
                if (buffer[bytes_read - 1] == '\n')
                    break;
                if (bytes_read >= size / 2)
                {
                    char *temp = realloc(buffer, sizeof(char) * size * 2);
                    if (temp == NULL)
                    {
                        free(buffer);
                        perror("Realloc Failed");
                        return 1;
                    }
                    buffer = temp;
                    size *= 2;
                }
            }

            if (write(1, buffer, bytes_read) == -1)
            {
                perror("write failed");
                free(buffer);
                return 1;
            }
            if (bytes_read == 0)
            {
                break;
            }
        }

        free(buffer);
        buffer = NULL;
        
    }

    else
    {

        struct stat dest;
        fstat(1, &dest);
        struct stat st;
        int index = 1;
        for (; index < argc; index++)
        {
            if (stat(argv[index], &st) == -1)
            {
                perror(argv[index]);

                continue;
            }
            if (S_ISDIR(st.st_mode))
            {
                fprintf(stderr, "%s: is a directory\n", argv[index]);
                continue;
            }
            int src_fd = open(argv[index], O_RDONLY);
            if (src_fd == -1)
            {
                perror(argv[index]);

                continue;
            }
            if ((st.st_dev == dest.st_dev) &&
                (st.st_ino == dest.st_ino))
            {
                write(2, "Input and Destination our same.Skipping it and moving ahead with rest of inputs\n", 81);
                continue;
            }
            int bytes = 0;
            char buffer[128];
            while ((bytes = read(src_fd, buffer, 128)) > 0)
            {
                if (write(1, buffer, bytes) == -1)
                {
                    perror("write failed");

                    break;
                }
            }

            close(src_fd);
        }
        return 0;
    }
}