#include "imports.c"


void ls(int argc, char *argv[]){
int flags[4];

for(int i=0;i<4;i++)
flags[i]=0;


if(argc==2 || argc ==3){
    char *p;
 if(argc==3){
p=argv[1];
 }
 else if(argv[1][0]=='-'){
    p=argv[1];
 }
 
 if(p!=NULL){
 
for(int i=1;p[i]!='\0';i++){
   
    if(p[i]=='a') flags[1]=1;

}
 }
 
}



DIR *d;

d=opendir(".");
if(argc>3){
 printf("\033[31m");
      printf("NO SUCH PATH/DIRECTORY EXISTS\n");
      printf("\033[m");
      return;
}
else if(argc==3){

    d=opendir(argv[2]);
    

    
    
}
else if(argc==2){
    if(argv[1][0]!='-')
    d=opendir(argv[1]);
}
if(d==NULL){
    printf("\033[31m");
      printf("NO SUCH PATH/DIRECTORY EXISTS\n");
      printf("\033[m");
      return;
}
struct dirent * reader;

while ((reader= readdir(d)) != NULL) 
{
    if(flags[1]==0){
 if( reader->d_name[0]=='.') continue;
    }
  
        printf("%s  ", reader->d_name);
   
    }
printf("\n");
   
    closedir(d);

}

int main(int argc, char *argv[]){
  
   printf("\033[33m");
    ls(argc ,argv);
     printf("\033[m");
  return 0;
}
