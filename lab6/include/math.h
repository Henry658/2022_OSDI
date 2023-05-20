#ifndef _MATH_H
#define _MATH_H

#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))

double pow(double x, double y);

int min(int x, int y);

int max(int x, int y);

#endif
