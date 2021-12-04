#include<stdio.h>

int main(){
	int x,y;
	float z1, z2, z3 ,z4,z5; 
	scanf("%i", &x);
	//scanf("%i", &y);
	z1 = x * y ; 
	z2 = x / y ; 
	z3 = x % y ; 
	z4 = x + y ; 
	z5 = x - y;
	printf("z1 = %f \n",z1);
	printf("z2 = %f \n",z2);
	printf("z3 = %f \n",z3);
	printf("z4 = %f \n",z4);
	printf("z5 = %f \n",z5);
}