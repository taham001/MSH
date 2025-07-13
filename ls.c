#include "imports.h"


void ls(int argc, char *argv[])
{
    int flags[4];

    for (int i = 0; i < 4; i++)
        flags[i] = 0;

    if (argc == 2 || argc == 3)
    {
        char *p = NULL;
        if (argc == 3)
        {
            p = argv[1];
        }
        else if (argv[1][0] == '-')
        {
            p = argv[1];
        }

        if (p != NULL)
        {

            for (int i = 1; p[i] != '\0'; i++)
            {

                if (p[i] == 'a')
                    flags[1] = 1;
            }
        }
    }

    char *target = ".";

    if (argc > 3)
    {
        fprintf(stderr, "Too many arguments\n");
        return;
    }
    else if (argc == 3)
    {
        target = argv[2];
    }
    else if (argc == 2)
    {
        if (argv[1][0] != '-')
        {

            target = argv[1];
        }
    }
    DIR *d;
    d = opendir(target);
    if (d == NULL)
    {

        perror(target);

        return;
    }
    struct dirent *reader;

    while ((reader = readdir(d)) != NULL)
    {
        if (flags[1] == 0)
        {
            if (reader->d_name[0] == '.')
                continue;
        }

        write(1, reader->d_name, str_len(reader->d_name));
        write(1, "  ", 2);
    }

    closedir(d);
    write(1, "\n", 1);
}

int main(int argc, char *argv[])
{

    ls(argc, argv);

    return 0;
}
