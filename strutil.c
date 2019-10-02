/* strutil.c  --- Implementation of strutil.h ---  Kaan B Erdogmus, CIS 380, kaanberk*/
#include "strutil.h"

#include <limits.h>  //for INT_MAX, INT_MIN (for portability)

int streq(const char * s1, const char * s2) {
    return (*s1 && *s2) ? (*s1 == *s2 && streq(s1 + 1, s2 + 1)) : !(*s1 || *s2);
}

int isspace(int c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

int atoi(const char * str) {
    while (isspace(*str)) str++;
    int sgn = (*str == '+') ? str++, 1 : (*str == '-') ? str++, -1 : 1;
    if (*str < '0' || *str > '9') return 0;

    int num = sgn * (*str++ - '0');
    while (*str >= '0' && *str <= '9') {
        if (10 * (long)num >= INT_MAX) return INT_MAX;
        if (10 * (long)num <= INT_MIN) return INT_MIN;

        num *= 10;

        if (num + (long)sgn * (*str - '0') >= INT_MAX) return INT_MAX;
        if (num + (long)sgn * (*str - '0') <= INT_MIN) return INT_MIN;

        num += sgn * (*str++ - '0');
    }
    return num;
}