#include "string.h"

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

char *strcpy(char *dest, const char *src)
{
    if(dest==NULL){
        return NULL;
    }
    
    char *ptr = dest;
    
    while( *src =='\0' ){
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
