#include "imports.c"




#define COMMAND_BUFFER_SIZE 1024

typedef struct tokens{
    char *args[30];
    int len;
    
}tokens;

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
char *path=malloc(sizeof(char)*(size1+size2));

p=base;
 while(*p!='\0'){
path[index++]=(*p);
p++;
 }
p=comm ;
while(*p!='\0'){
    path[index++]=*p;
    p++;
}
path[index++]='\0';

return path;
}
tokens* splitInput(char  *buffer,int index){
    tokens * tok=malloc(sizeof(tokens));
 tok->len=0;
    char *start=buffer;
for(int i=0;i<index;i++){
    
   
    if( buffer[i]==' ' ){
        buffer[i]='\0';
        tok->args[tok->len]=start;
        tok->len++;
        start=&buffer[i+1];
    
}
}
tok->args[tok->len]=start;
tok->len++;
for (int i = tok->len; i < 30; i++) {
    tok->args[i] = NULL;
}

return tok;
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
void handleCommand(tokens* tok){
   
    if(tok->args[0][0]=='\0') {return;}
    if (compare(tok->args[0], "exit") == 1) {
        printf("Exiting shell...\n");
        free(tok);
        tok=NULL;
        exit(0);  
    }
    if(compare(tok->args[0],"cd")==1){
       int err=chdir(tok->args[1]);
       if(err){
        printf("NO SUCH DIRECTORY EXISTS\n");
        return;
       }
       return;
    }
   if(compare(tok->args[0],"mkdir")==1){
       if (mkdir(tok->args[1], 0755) == -1) {
        perror("mkdir failed");
    } else {
        printf("Directory created.\n");
    }
    return ;
}
    

char *path=copy("/bin/MSH/",tok->args[0]);

 int id=fork();
if(id==0){
   
    
    int success=1;
   
   success=  execv(path,tok->args );
   
  if(success==-1){
 free(path);

    exit(1);
  }
}

else{
     int status;
    wait(&status);
    if(status!=0){
        printf("\033[31m");
      printf("NO SUCH COMMAND EXISTS\n");
      printf("\033[m");
    }
   
   
   
    
   
}
free(path);


}
void prompt(){
    printf("Welcome to MSH shell,This shell follows posix standard\n");

}
int main(){
 char buffer[COMMAND_BUFFER_SIZE];


prompt();
while(1){
char c;
    int index=0;
    char prev='[';
    while(scanf("%c",&c)==1 && c!='\n'){
if(index==0 && c==' ')continue;
        if(c==' '){
            prev=' ';
            continue;
        }
        if(prev==' '){
            buffer[index++]=prev;
            prev='[';
        }
        buffer[index++]=c;
    }
buffer[index]='\0';


tokens *tok=splitInput(buffer,index);



handleCommand(tok);


free(tok);

tok=NULL;
}


    return 0;
}