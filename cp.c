#include "imports.h"


#define BUFF_SIZE 128

int copyFiles(const char *src, int dest_fd)
{
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0)
    {
        perror(src);
        return 1;
    }

    char buffer[BUFF_SIZE];
    int bytes_read = 0;
    while ((bytes_read = read(src_fd, buffer, BUFF_SIZE)) > 0)
    {
        int bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_read != bytes_written)
        {
            fprintf(stderr, "Error writing to destination file\n");
            close(src_fd);
            return 1;
        }
    }

    if (bytes_read < 0)
    {
        fprintf(stderr, "Error reading from %s\n", src);
    }

    close(src_fd);
    return 0;
}

int copyDirectory(char *src, char *destDir)
{
    DIR *s = opendir(src);
    if (!s)
    {
        perror(src);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(s)) != NULL)
    {
        if (compare(entry->d_name, ".", 1) == 1 || compare(entry->d_name, "..", 2) == 1){
            continue;
        }

        char *src_path = append_str(src, "/");
        char *src_full = append_str(src_path, entry->d_name);
        free(src_path);

        struct stat st;
        if (stat(src_full, &st) == -1)
        {
            perror(src_full);
            free(src_full);
            continue;
        }

        char *dest_path = append_str(destDir, "/");
        char *dest_full = append_str(dest_path, entry->d_name);
        free(dest_path);

        if (S_ISDIR(st.st_mode))
        {
            if (mkdir(dest_full, 0755) == -1 && errno != EEXIST)
            {
                perror(dest_full);
                free(dest_full);
                free(src_full);
                closedir(s);
                return 1;
            }
            if (copyDirectory(src_full, dest_full) != 0)
            {
                free(dest_full);
                free(src_full);
                closedir(s);
                return 1;
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            int dest_fd = open(dest_full, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (dest_fd < 0)
            {
                perror(dest_full);
                free(dest_full);
                free(src_full);
                closedir(s);
                return 1;
            }
            if (copyFiles(src_full, dest_fd) != 0)
            {
                free(dest_full);
                free(src_full);
                close(dest_fd);
                closedir(s);
                return 1;
            }
            close(dest_fd);
        }

        free(dest_full);
        free(src_full);
    }

    closedir(s);
    return 0;
}

void cpDirectories(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Not enough arguments provided\n");
        return;
    }

    struct stat dest;
    if (stat(argv[argc - 1], &dest) == -1)
    {
        perror(argv[argc - 1]);
        return;
    }

    DIR *dir = opendir(argv[argc - 1]);
    if (!dir)
    {
        perror(argv[argc - 1]);
        return;
    }

    for (int i = 2; i < argc - 1; i++)
    {
        struct stat st;
        if (stat(argv[i], &st) == -1)
        {
            perror(argv[i]);
            continue;
        }

        char *dst_path = append_str(argv[argc - 1], "/");
        char *dst_full = append_str(dst_path, argv[i]);
        free(dst_path);

        if (S_ISDIR(st.st_mode))
        {
            if ((st.st_dev == dest.st_dev) && (st.st_ino == dest.st_ino))
            {
                free(dst_full);
                continue;
            }

            if (mkdir(dst_full, 0755) == -1 && errno != EEXIST)
            {
                perror(dst_full);
                free(dst_full);
                continue;
            }

            if (copyDirectory(argv[i], dst_full) != 0)
            {
                free(dst_full);
                continue;
            }
        }
        else
        {
            int dest_fd = open(dst_full, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (dest_fd < 0)
            {
                perror(dst_full);
                free(dst_full);
                continue;
            }
            if (copyFiles(argv[i], dest_fd) != 0)
            {
                close(dest_fd);
                free(dst_full);
                continue;
            }
            close(dest_fd);
        }

        free(dst_full);
    }

    closedir(dir);
}

void cpFiles(int argc, char *argv[])
{
    struct stat dest;
    int argNo = 1;

    int dest_fd;
    int dest_exists = stat(argv[argc - 1], &dest) != -1;

    if (dest_exists && S_ISDIR(dest.st_mode))
    {
        fprintf(stderr, "%s is a directory. Use -r for recursive copy\n", argv[argc - 1]);
        return;
    }

    dest_fd = open(argv[argc - 1], O_WRONLY | O_CREAT | (dest_exists ? O_APPEND : O_TRUNC), 0644);
    if (dest_fd < 0)
    {
        perror(argv[argc - 1]);
        return;
    }

    for (int i = argNo; i < argc - 1; i++)
    {
        struct stat src;
        if (stat(argv[i], &src) == -1)
        {
            perror(argv[i]);
            continue;
        }

        if (S_ISDIR(src.st_mode))
        {
            fprintf(stderr, "%s is a directory. Use -r to copy directories\n", argv[i]);
            continue;
        }

        if (dest_exists && (src.st_dev == dest.st_dev && src.st_ino == dest.st_ino))
        {
            continue;
        }

        if (copyFiles(argv[i], dest_fd) != 0)
        {
            continue;
        }
    }

    close(dest_fd);
}
int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr,"Less files specified\n");
        return 1;
    }
    int recur = 0;
    if (compare(argv[1], "-r", 2) == 1)
    {
       
        recur = 1;
    }

    else if (argv[1][0] == '-')
    {
        fprintf(stderr,"Wrong format! If you want to recursively copy use -r flag\n");
        return 1;
    }

    if (!recur)
    {
        cpFiles(argc, argv);
    }
    else
        cpDirectories(argc, argv);

    return 0;
}

