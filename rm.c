#include "imports.h"


int removed(char *path, int flags[])
{

    struct stat path_stat;

    if (stat(path, &path_stat) == -1)
    {

        perror(path);
        return 1;
    }
    if (S_ISREG(path_stat.st_mode) || S_ISLNK(path_stat.st_mode))
    {
        if (flags[1])
        {
            write(1, "Removing File: ", 16);
            write(1, path, str_len(path));
            write(1, "\n", 1);
        }
        if (unlink(path) != 0)
        {
            perror(path);
            return 1;
        }
        return 0;
    }

    if (S_ISDIR(path_stat.st_mode))
    {
        DIR *dir = opendir(path);
        if (!dir)
        {

            perror(path);
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (compare(entry->d_name, ".", 1) == 1 || compare(entry->d_name, "..", 2) == 1)
                continue;
            if (!flags[0])
            {
                fprintf(stderr, "Use -f to remove non-empty directories\n");
                closedir(dir);
                return 1;
            }

            char *temp = append_str(path, "/");
            char *full_path = append_str(temp, entry->d_name);

            if (removed(full_path, flags) != 0)
            {
                free(temp);
                free(full_path);
                closedir(dir);
                return -1;
            }
            free(temp);
            free(full_path);
        }

        closedir(dir);

        if (flags[1])
        {
            write(1, "Removing directory: ", 21);
            write(1, path, str_len(path));
            write(1, "\n", 1);
        }
        if (rmdir(path) != 0)
        {

            perror(path);
            return -1;
        }
        return 0;
    }
    return 0;
}
int main(int argc, char *argv[])
{

    if (argc == 1)
    {
        fprintf(stderr, "NO file/directory specified\n");
        return 0;
    }
    else
    {

        char *path = argv[1];
        int flags[2];
        flags[0] = 0; // flag for recursive deletion
        flags[1] = 0; // flag for verbose printing
        int f = 0;
        if (argc >= 3)
        {
            path = argv[2];
            char *p = argv[1];
            if (*p == '-')
            {
                f = 1;
                p++;
                while (*p != '\0')
                {

                    if (*p == 'f')
                        flags[0] = 1;
                    else if (*p == 'v')
                        flags[1] = 1;
                    else
                    {

                        fprintf(stderr, "Usage: rm [-vf] <file_or_directory> ...\n");
                        return 1;
                    }
                    p++;
                }
            }
        }
        int index = 1;
        if (f == 1)
        {
            index = 2;
        }
        int try = 0;
        while (index < argc)
        {
            path = argv[index];
            if (removed(path, flags) == 1)
                try = 1;

            index++;
        }
        return try;
    }

    return 0;
}