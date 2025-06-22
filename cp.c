#include "imports.c"
#define BUFF_SIZE 128
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
 if (index > 0 && path[index-1] != '/')
    path[index++] = '/';

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

void copyFiles(char *src,int dest_fd){
    
    int src_fd=open(src,O_RDONLY);
 
 if(src_fd<0){
    printf("File %s does not have reading permissions\n",src);
    
    return;
 }
 
char buffer[BUFF_SIZE];
    int bytes_read=0;;
  
   
       
    while((bytes_read=read(src_fd,buffer,BUFF_SIZE))>0){
  
        int bytes_written=write(dest_fd,buffer,bytes_read);
        if(bytes_read!=bytes_written){
            printf("Error writing to destination file\n");
            
             close(src_fd);
             return;

        }
    }
    
     if (bytes_read < 0) {
            printf( "Error reading from %s\n", src);
         
           
        }
    


   
 close(src_fd);
  




}
int copyDirectory(char *src,char* dirPath){
    struct stat st;
    DIR * s=opendir(src);
      struct dirent *entry;
    if(s==NULL){
        printf("Reading permission for %s denied",src);
        return -1;
    }
    while ((entry = readdir(s)) != NULL) {
       if (compare(entry->d_name, ".") == 1 || compare(entry->d_name, "..") == 1)
            continue;
        char *src_path=copy(src,entry->d_name);
        if (stat(src_path, &st) == -1) {
           printf("Permission denied for %s",src_path);
           free(src_path);
            continue;
        }
         if (S_ISDIR(st.st_mode)) {
            char *destDir=copy(dirPath,entry->d_name);
            mkdir(destDir, 0755);
            
            copyDirectory(src_path,destDir);
            free(destDir);
           
            
        } else if (S_ISREG(st.st_mode)) {
         char *dest_path=copy(dirPath,entry->d_name);
         int dest_fd=open(dest_path,  O_WRONLY | O_CREAT | O_TRUNC, 0644);
            copyFiles(src_path,dest_fd);
            close(dest_fd);
            free(dest_path);
        }
        free(src_path);
    }

    return 1;


}

void cpDirectories(int argc ,char*argv[]){
    if(argc<4){
        printf("Not enough arguments provided\n");
        return;
    }
    struct stat st;
    
    
    
    int argNo=2;
     DIR *dir = opendir(argv[argc-1]);
    if (dir == NULL) {
   printf("Directory %s does not exists",argv[argc-1]);
   return;
    }
    while(argNo<(argc-1)){
        if(stat(argv[argNo],&st)==-1){
            printf("Directory/File %s does not exists\n",argv[argNo]);
            closedir(dir);
            return;
        }
         if (S_ISDIR(st.st_mode)){
            if(copyDirectory(  argv[argNo],argv[argc-1])==-1){
                closedir(dir);
                return;
            }
       
       }
       else{
        char *dest=copy(argv[argc-1],argv[argNo]);
         int dest_fd = open(dest,  O_WRONLY | O_CREAT | O_TRUNC, 0644);
 if(dest_fd <0){
    printf("File %s doe not having writing permissions\n",argv[argc-1]);
    free(dest);
  closedir(dir);
    return;
 }
        copyFiles(argv[argNo],dest_fd);
        free(dest);
        
       }

      
        argNo++;
    }
    

closedir(dir);
    
}

void cpFiles(int argc,char*argv[]){
    struct stat st;
  

 int argNo=1;
 
 if (stat(argv[argc-1], &st) ==-1) {
    printf("No %s exisits\n",argv[argc-1]);
return;}
    if(stat(argv[argc-1], &st) !=-1){
         if (S_ISDIR(st.st_mode)){
        printf("%s is a directory.Copy not valid.Use -r to copy directory\n",argv[argc-1]);
     
        return;

       }
    }


    int dest_fd = open(argv[argc-1],  O_WRONLY | O_CREAT | O_TRUNC, 0644);
 if(dest_fd <0){

    printf("File %s doe not having writing permissions\n",argv[argc-1]);
  
    return;
 }
 while(argNo<(argc-1)){
   if (stat(argv[argNo], &st) == -1) {
        printf("File %s does not exists\n",argv[argNo]);
        close(dest_fd);
        return;
    }
       if (S_ISDIR(st.st_mode)){
        printf("%s is a directory.Copy not valid.Use -r to copy directory\n",argv[argNo]);
        close(dest_fd);
        return;

       }
    
copyFiles(argv[argNo],dest_fd);
write(dest_fd,"\n",1);

    argNo++;
 }
 close(dest_fd);

 
 

    


}

int main(int argc, char *argv[]){
if(argc<3){
    printf("Less files specified\n");
    return 0;
}
int recur=0;
if(compare(argv[1],"-r")){
recur=1;
}

else if(argv[1][0]=='-'){
    printf("Wrong format! If you want to recursively copy use -r flag\n");
    return 0;
}
if(!recur)
 cpFiles(argc,argv);
else cpDirectories(argc,argv);

   
    return 0;
}