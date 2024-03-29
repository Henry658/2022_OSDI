#include "printf.h"
#include "math.h"

char *itox(int value, char *s){ // int to hex 
	int idx = 0;
	
	char temp[8 + 1];
	int tidx = 0;
	while(value){
		int r = value % 16;
		if (r < 10){
			temp[tidx++] = '0' + r;
		}
		else{
			temp[tidx++] = 'a' + r - 10;
		}
		value /= 16;
	}
		
	int i;
	for(i = tidx - 1; i >= 0 ; i--){
		s[idx++] = temp[i];
	}
	s[idx] = '\0';

	return s;
}

char *itoa(int value, char *s){
	int idx = 0;
	if(value < 0){
		value *= -1;
		s[idx++] = '-';
	}

	char temp[10];
	int tidx = 0;
	do{
		temp[tidx++] = '0' + value % 10;
		value /= 10;
	}while(value != 0 && tidx < 11);

	int i;
	for(i = tidx - 1; i >= 0; i--){
		s[idx++] = temp[i];
	}
	s[idx] = '\0';

	return s;
}

char *ftoa(float value, char *s){
	int idx = 0;
	if(value < 0){
		value = -value;
		s[idx++] = '-';
	}

	int ipart = (int)value;
	float fpart = value - (float)ipart;

	char istr[11];
	itoa(ipart, istr);

	char fstr[7];
	fpart *= (int)pow(10, 6);
	itoa((int)fpart, fstr);

	char *ptr = istr;
	while(*ptr) s[idx++] = *ptr++;
	s[idx++] = '.';

	ptr = fstr;
	while(*ptr) s[idx++] = *ptr++;
	s[idx++] = '\0';
	return s;
}

unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args){
	char *dst_orig = dst;
	
	while(*fmt){
		if(*fmt == '%'){
			fmt++;
			// escape %
			if(*fmt == '%'){
				goto put;
			}
			// string
			if(*fmt == 's'){
				char *p = __builtin_va_arg(args, char *);
				while(*p){
					*dst++ = *p++;
				}
			}
			// number
			if(*fmt == 'd'){
				int arg = __builtin_va_arg(args, int);
				char buf[11];
				char *p = itoa(arg, buf);
				while(*p){
					*dst++ = *p++;
				}
			}
			// hex
			if(*fmt =='x'){
				int arg = __builtin_va_arg(args, int);
				char buf[8 + 1];
				char *p = itox(arg, buf);
				while(*p){
					*dst++ = *p++;
				}
			}
			// float
			if(*fmt == 'f'){
				float arg = (float)__builtin_va_arg(args, double);
				char buf[19]; // sign + 10 int + dot + 6 float
				char *p = ftoa(arg, buf);
				while(*p){
					*dst++ = *p++;
				}
			}
		}
		else{
			put:
				*dst++ = *fmt;
		}
		fmt++;
	}
	*dst = '\0';

	return dst - dst_orig;
}

unsigned int sprintf(char *dst, char *fmt, ...){
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	return vsprintf(dst, fmt, args);
}

