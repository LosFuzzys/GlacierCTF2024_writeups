#include <stdio.h>

int main()
{
    char* flag_str = {
        #include "flag.txt"
    };

    printf("Flag: %s\n", flag_str);

    return 0;
}