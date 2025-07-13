
#include "imports.h"


#define BUFF_SIZE 128




void copyFiles(char *src, int dest_fd)
{

    int src_fd = open(src, O_RDONLY);

    if (src_fd < 0)
    {
        fprintf(stderr,"File %s does not have reading permissions\n", src);

        return;
    }

    char buffer[BUFF_SIZE];
    int bytes_read = 0;
    

    while ((bytes_read = read(src_fd, buffer, BUFF_SIZE)) > 0)
    {

        int bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_read != bytes_written)
        {
            fprintf(stderr,"Error writing to destination file\n");

            close(src_fd);
            return;
        }
    }

    if (bytes_read < 0)
    {
        fprintf(stderr,"Error reading from %s\n", src);
    }

    close(src_fd);
}
int copyDirectory(char *src, char *dirPath)
{
    struct stat st;
    DIR *s = opendir(src);
    struct dirent *entry;
    if (s == NULL)
    {
        perror(src);
        return -1;
    }
    while ((entry = readdir(s)) != NULL)
    {
        if (compare(entry->d_name, ".", 1) == 1 || compare(entry->d_name, "..", 2) == 1){
            continue;
        }
            char * temp=append_str(src,"/");
        char *src_path = append_str(temp, entry->d_name);
        if (stat(src_path, &st) == -1)
        {
            perror(src_path);
            free(src_path);
            continue;
        }
        if (S_ISDIR(st.st_mode))
        {
            char *temp_=append_str(dirPath,"/");
            char *destDir = append_str(temp_, entry->d_name);
            mkdir(destDir, 0755);

            copyDirectory(src_path, destDir);
            free(destDir);
            free(temp_);
        }
        else if (S_ISREG(st.st_mode))
        {
            char *temp_=append_str(dirPath,"/");
            char *dest_path = append_str(temp_, entry->d_name);
            int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            copyFiles(src_path, dest_fd);
            close(dest_fd);
            free(dest_path);
            free(temp_);
        }
        free(src_path);
        free(temp);
    }

    return 1;
}

void cpDirectories(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr,"Not enough arguments provided\n");
        return ;
    }
    struct stat dest;

    stat(argv[argc - 1], &dest);

    int argNo = 2;
    DIR *dir = opendir(argv[argc - 1]);
    if (dir == NULL)
    {

        if (mkdir(argv[argc - 1], 0755) != 0)
        {
            perror(argv[argc-1]);
            return;
        }

        dir = opendir(argv[argc - 1]);
    }
    struct stat st;
    while (argNo < (argc - 1))
    {
        if (stat(argv[argNo], &st) == -1)
        {
            perror(argv[argNo]);
            closedir(dir);
            return;
        }
        if (S_ISDIR(st.st_mode))
        {
            char * temp=append_str(argv[argc-1],"/");
            char *dest_dir = append_str(temp, argv[argNo]);
            mkdir(dest_dir, 0755);
            if ((st.st_dev == dest.st_dev) &&
                (st.st_ino == dest.st_ino))
            {
                argNo++;
                continue;
            }
            if (copyDirectory(argv[argNo], dest_dir) == -1)
            {
                free(temp);
                free(dest_dir);
                closedir(dir);
                return;
            }
            int id = fork();
            if (id == 0)
            {

                char *ptr[] = {"rm", "-f", argv[argNo], NULL};

                int success = 1;

                success = execv("/bin/MSH/rm", ptr);

                if (success == -1)
                {

                    exit(1);
                }
            }
            else
            {
                int status;
                wait(&status);
                if (status != 0)
                {
                    perror("Move failed\n");
                    closedir(dir);
                    return;
                }
            }
        }
        else
        {
            char * temp=append_str(argv[argc-1],"/");
            char *dest = append_str(temp, argv[argNo]);
            int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (dest_fd < 0)
            {
                perror(argv[argc-1]);
                free(dest);
                free(temp);
                closedir(dir);
                return;
            }
            copyFiles(argv[argNo], dest_fd);
            if (remove(argv[argNo]) != 0)
            {
               perror(argv[argNo]);
               free(temp);
                free(dest);
                closedir(dir);
                return;
            }
            free(dest);
            free(temp);
        }

        argNo++;
    }

    closedir(dir);
}

void cpFiles(int argc, char *argv[])
{
    struct stat dest;

    int argNo = 1;

    int dest_fd;
    if (stat(argv[argc - 1], &dest) != -1)
    {

        if (S_ISDIR(dest.st_mode))
        {
            fprintf(stderr,"%s is a directory. Given operation not valid.Use -r for directories\n", argv[argc - 1]);

            return;
        }

        dest_fd = open(argv[argc - 1], O_WRONLY | O_APPEND);
    }

    else
        dest_fd = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0)
    {

        perror(argv[argc-1]);

        return;
    }
    struct stat src;

    while (argNo < (argc - 1))
    {
        if (stat(argv[argNo], &src) == -1)
        {
            perror(argv[argNo]);
            close(dest_fd);
            return;
        }
        if (S_ISDIR(src.st_mode))
        {
            fprintf(stderr,"%s is a directory.Copy not valid.Use -r to copy directory\n", argv[argNo]);
            close(dest_fd);
            return;
        }
        if ((src.st_dev == dest.st_dev) &&
            (src.st_ino == dest.st_ino))
        {
            argNo++;
            continue;
        }

        copyFiles(argv[argNo], dest_fd);
        if (remove(argv[argNo]) != 0)
        {
            perror(argv[argNo]);
            close(dest_fd);
            return;
        }
      

        argNo++;
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
        fprintf(stderr,"Wrong format! If you want to recursively move use -r flag\n");
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