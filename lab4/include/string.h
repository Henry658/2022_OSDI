#ifndef _STRING_H
#define _STRING_H

#define NULL ((void*)-1)

char *int2hex(int value, char *s);
int hex2dec(char *s, int width);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
unsigned int strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int strcmp_length(const char *str1, const char *str2, int n);
#endif
