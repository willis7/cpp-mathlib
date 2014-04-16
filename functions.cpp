/* Miscellaneous useful functions */

#include "OR.h"

//Returns the lowest power of two that is higher than the input
int LowestPowerTwo(int in)
{
	int x=1;
	while(x<in) x = x<<1;
	return x;
}


bool IsPowerOfTwo(int n)
{
	if(n<1) return false;
    return !(n & (n - 1)); //this checks if the integer n is a power of two or not
}

//Tells what power of two an input is, -1 if it's not a power
int WhichPowerOfTwo(int n)
{
	int p = 0;
	if(!IsPowerOfTwo(n)) return -1;
	while(n!=1)
	{
		n = n>>1;
		++p;
	}
	return p;
}

//Best code ever
float magic_sqrt(float number)
{
   long i;
   float f = 1.5f, x = number*0.5f, y = number;
   i = *(unsigned long*)&y;
   i = 0x5f3759df - (i>>1);
   y = *(float*)&i;
   y = y*(f - x*y*y);
   //y = y * (f - x*y*y);		//Repeated iterations improve the quality of the result - not needed here
   return number*y;
}

//Best code ever
float magic_inv_sqrt(float number)
{
   long i;
   float f = 1.5f, x = number*0.5f, y = number;
   i = *(unsigned long*)&y;
   i = 0x5f3759df - (i>>1);
   y = *(float*)&i;
   y = y*(f - x*y*y);
   y = y*(f - x*y*y);		//Repeated iterations improve the quality of the result
   return y;
}

float randf()
{
	return (float)rand()/(float)RAND_MAX;
}

float randlim(float min, float max)
{
	float range = max - min;
	return min + range*(float)rand()/(float)RAND_MAX;
}

Fsincos_t fsincos(float x)
{
	Fsincos_t tmp;

	__asm
	{
		fld x				// load x onto st0
		fsincos
		fstp tmp.cosine		// return and pop cosine from st1
		fstp tmp.sine		// return and pop sine from st0
	}

	return tmp;
}