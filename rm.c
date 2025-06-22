#include "imports.c"



char * copy(char*base ,char *comm){
  
   
    char *p=base;
    int size1=0;
    while(*p!='\0'){
size1++;
p++;
    }
    size1++;
    p=comm;
    int size2=0;
    while(*p!='\0'){
        size2++;
        p++;
    }
size2++;
int index=0;
char *path=malloc(sizeof(char)*(size1+size2+1));

p=base;
 while(*p!='\0'){
path[index++]=(*p);
p++;
 }
 path[index++]='/';
p=comm ;
while(*p!='\0'){
    path[index++]=*p;
    p++;
}
path[index++]='\0';

return path;
}
int compare(char *p1,char*p2){
    
while(*p1!='\0' && *p2!='\0'){
    if(*p1!=*p2)
    return 0;
    p1++;
    p2++;
}
if(*p1!='\0' || *p2!='\0') return 0;
return 1;
}
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

int removed(char *path, int flags[]){
  
     struct stat path_stat;

    if (stat(path, &path_stat) != 0) {
      
        printf("Unable to fetch info\n");
        return 1;
    }
if (S_ISREG(path_stat.st_mode) || S_ISLNK(path_stat.st_mode)) {
    if (flags[1]) {
        printf("Removing file: %s\n", path);
    }
    if (unlink(path) != 0) {
       printf("Removing failed\n");
       return 1;
        }
        return 0;
    }
    

if (S_ISDIR(path_stat.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) {
           
                printf("opendir failed\n");
                return 1;
            
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if(flags[0]==0){
                printf("Directory Not Empty\n");
                return -1;
            }
            
            if (compare(entry->d_name, ".") == 1 || compare(entry->d_name, "..") == 1)
                continue;

           char *full_path=copy(path,entry->d_name);
            

            if (removed(full_path,flags ) != 0 ) {
                closedir(dir);
                return -1;
            }
            free(full_path);
        }

        closedir(dir);


        if (flags[1]) {
            printf("Removing directory: %s\n", path);
        }
        if (rmdir(path) != 0) {
            if (!flags[0]) {
                printf("rmdir failed");
                return -1;
            }
        }
        return 0;
    }

}
int main(int argc,char*argv[]){

if(argc==1){
    printf("NO file/directory specified\n");
    return 0;
}
else{
    char *path=argv[1];
    int flags[2];
    flags[0]=0;flags[1]=0;
    if(argc==3){
        path=argv[2];
        char *p=argv[1];
        p++;
        while(*p!='\0'){
            
           if(*p=='f') flags[0]=1;
                else if(*p=='v') flags[1]=1;
            else{
            
                exit(1);
            }
            p++;
        }

    }
   
   removed(path,flags);
}

return 0;



}