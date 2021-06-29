#include "./helper.h"

// return the number of digits
int countDigits(int num)
{
    if (num == 0)
        return 1;

    int count = 0;

    while (num != 0)
    {
        num /= 10; // n = n/10
        ++count;
    }

    return count;
}

// https://codeforwin.org/2016/02/c-program-to-find-maximum-and-minimum-using-functions.html
int min(int num1, int num2)
{
    return num1 < num2 ? num1 : num2;
}
