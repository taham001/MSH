#include "imports.h"


int main(int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        char *p = argv[i];
        write(1,p, str_len(p));
        write(1," ",1);
    }
    write(1,"\n",1);
    return 0;
}