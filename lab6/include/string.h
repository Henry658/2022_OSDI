#ifndef _STRING_H
#define _STRING_H

#define NULL ((void*)-1)

char *int2hex(int value, char *s);

int hex2dec(char *s,int width);

char *strcpy(char *dest, const char *src);

char *strcat(char *dest, const char *src);

char *strchr(register const char *s, int c);

unsigned int strlen(const char *str);

int strcmp(const char *str1, const char *str2);

int strcmp_len(const char * str1, const char *str2, int n);

unsigned int swap_endian_uint32(unsigned int num);

int swap_endian_int32(int num);

void reverse_str(char *s);

void ltoxstr(long long x, char str[]);

//typedef unsigned long uint32_t;

//void uitoxstr( uint32_t x, char str[]);

void itoxstr(int x, char str[]);

int atoi(char *str);
#endif
