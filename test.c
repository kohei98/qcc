#include <stdio.h>

int fib(int x)
{
    if (x <= 0)
        return 1;
    else
    {
        return fib(x - 1) + fib(x - 2);
    }
}