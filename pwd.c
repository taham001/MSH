#include "imports.h"

  void pwd(){
    char cwd[PATH_MAX];  
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
         
        write(1,"Current working directory:",27);
        write(1,cwd,str_len(cwd));
        write(1,"\n",1);
      
    } else {
        perror("Could Not Be Found");
       
    }
}
int main(){
  pwd();
  return 0;
}
