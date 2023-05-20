#include "math.h"

double pow(double x, double y){
	if(y==0){
		return 1;
	}
	return x * pow(x, y-1);
}

int min(int x, int y){
	if(x <= y){
		return x;
	}
	return y;
}

int max(int x, int y){
	if(x > y){
		return x;
	}
	return y;
}
