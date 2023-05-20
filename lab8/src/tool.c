#include "tool.h"
int memcmp(void* s1, void* s2, int n){
    unsigned char *a = s1, *b = s2;
    while(n--){
        if( *a != *b ){
            return *a -*b;
        }
        a++;
        b++;
    }
    return 0;
}
