/* strutil.h  --- Utility functions for string manipulation  ---  Kaan B Erdogmus, CIS 380, kaanberk*/

#ifndef STRUTIL_H
#define STRUTIL_H

#undef streq
#undef isspace
#undef atoi

/* compares s1 to s2
 * returns 1 of s1 is same string as s2, 0 otherwise
 */
int streq(const char * s1, const char * s2);

/* checks whether a character is a whitespace character 
 * (space, form-feed, newline, carriage-return, horizontal tab, vertical tab) 
 */
int isspace(int c);

/* converts the first appearance of a number in a string to an int
 * starts reading at first digit or '+' or '-' and continues until last digit
 * if value is less than INT_MIN or more than INT_MAX returns INT_MIN or INT_MAX
 * returns 0 if invalid input
 */
int atoi(const char * str);

#endif
