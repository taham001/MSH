
#include "imports.h"
int str_len(char *p){
    int i=0;

    while(p[i]!='\0'){
        i++;
    }
    return i;
}
char * copy_str(char *p){
    char *str=malloc(sizeof(char)*(str_len(p)+1));
    int i=0;
while(p[i]!='\0'){
    str[i]=p[i];
    i++;
}
str[i]='\0';
return str;
}

int compare(char *p1,char*p2,int len){
    int i=0;

    
while(i<len && p1[i]!='\0' && p2[i]!='\0'){
    if(p1[i]!=p2[i]) return 0;
    i++;
}
if(i<len) return 0;

return 1;
}

char * append_str(char *p1,char*p2){
 int len1=str_len(p1);
 int len2=str_len(p2);
 char * appended=malloc(sizeof(char)*(len1+len2+1));
 int size=0;

int index=0;
while(p1[index]!='\0'){
    appended[size++]=p1[index];
    index++;
}
index=0;
while(p2[index]!='\0'){
    appended[size++]=p2[index];
    index++;
}
appended[size++]='\0';
return appended;
}