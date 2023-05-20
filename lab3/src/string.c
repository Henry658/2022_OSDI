#include "string.h"
typedef unsigned long uint32_t;
char *int2hex(int value, char *s){
    int idx = 0, i;
    
    char temp[8 + 1]; // 8 for 8 bit 1 for '\0'
    int temp_idx = 0;
    while(value){
        int r = value % 16;
        if(r>10){
            temp[temp_idx++] = 'a' + r -10;
        }
        else{
            temp[temp_idx++] = '0' + r ;
        }
        value /= 16;
    }
    
    //reverse
    for(i = temp_idx -1; i>=0 ; i--){
        s[idx++] = temp[i];
    }
    s[idx] = '\0';

    return s;
}

int hex2dec(char *s,int width){
    int temp = 0, i = 0 ;
    while(i<width){
        temp *= 16;
        if((*s - '0')>10){
            temp += (*s - '0') - 7;
        }
        else{
            temp += (*s - '0');
        }
        i++;
        s++;
    }
    return temp;

}



char *strcpy(char *dest, const char *src)
{
    if(dest==NULL){
        return NULL;
    }
    char *ptr = dest;
    
    while( *src !='\0' ){
        *dest = *src;
        dest++;
        src++;
    }
    
    *dest = '\0';

    return ptr;
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest + strlen(dest);
    
    while( *(src) != '\0' ){
        *ptr = *src++;   
    }
    
    *ptr = '\0';

    return dest; 

}

char *strchr(register const char *s, int c){
	do{
		if(*s == c){
			return (char *)s;
		}
	}while(*s++);
	return 0;

}

unsigned int strlen(const char *str){
    unsigned int size = 0;
    while(1){
        if( *(str+size) == '\0' )
            break;
        size++;
    }
    return size;
}

int strcmp(const char *str1, const char *str2){
    int i;
    for(i = 0; i < strlen(str1); i++){
        if( *(str1+i) != *(str2+i) ){
            return *(str1+i) - *(str2+i);
        }
    }
    return *(str1+i) - *(str2+i);
}

int strcmp_len(const char * str1, const char *str2, int n){
    int i;
    if (strlen(str1)<n)
    	return 1;
    for(i = 0; i < n; i++){
        if( *(str1+i) != *(str2+i) ){
        	return *(str1+i) - *(str2+i);
        }
    }
    return 0;
}

unsigned int swap_endian_uint32(unsigned int num){
	num = ((num << 8) & 0xFF00FF00) | ((num >> 8) & 0xFF00FF);
	return (num << 16) | (num >> 16);
}

int swap_endian_int32(int num){
	num = ((num << 8) & 0xFF00FF00) | ((num >> 8) & 0xFF00FF);
	return (num << 16) | ((num >> 16) & 0xFFFF);
}

void reverse_str(char *s){
	int i;
	char temp;
	for(i=0;i<strlen(s)/2;++i){
		temp = s[strlen(s)-1-i];
		s[strlen(s)-1-i] = s[i];
		s[i] = temp;
	}
}

void ltoxstr(long long x, char str[]){
	int i = 0;
	while(x){
		int temp = x % 16;
		if(temp > 9){
			str[i++] = temp - 10 + 'A';
		}
		else{
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}

typedef unsigned long uint32_t;

void uitoxstr( uint32_t x, char str[]){
	uint32_t i = 0;
	while(x){
		int temp = x % 16;
		if(temp > 9){
			str[i++] = temp - 10 + 'A';
		}
		else{
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}

void itoxstr(int x, char str[]){
	int i = 0;
	while(x){
		int temp = x % 16;
		if(temp > 9){
			str[i++] = temp - 10 + 'A';
		}
		else{
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}

int atoi(char *str){
	int res = 0;
	for(int i=0; str[i]!='\0'; i++){
		if(str[i]>'9'||str[i]<'0') return res;
		res = res * 10 + str[i] - '0';
	}
	return res;
}
