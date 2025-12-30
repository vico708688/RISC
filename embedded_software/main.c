#include <stdio.h>
#include <math.h>

void put_char(char c)
{
    *(char *)0x10000000 = c;
}

void put_string(const char *str)
{
    char c;
    while ((c = *str++) != '\0')
        put_char(c);
}

int main()
{
    put_string("Hello, World!, from put_string()\n");
    printf("Hello, World!, we are in %s.\n", __PRETTY_FUNCTION__);
    printf("PI: %g\n", M_PI);
    return 0;
}