//The font drawing routines

#include "OR.h"

extern GraphicalObject font;
extern ofstream dbout;
extern float screenratio;


int DrawString(GraphicalObject *usefont, const char *TextString, int x, int y, int limit, UINT colour)
{
	int x_dest, y_dest, i=0, ch, counter=0, wordwidth;
	int eighthcolour = (((colour&0xff000000)>>3)&0x1f000000) | (colour&0xffffff);
	int quarcolour   = (((colour&0xff000000)>>2)&0x3f000000) | (colour&0xffffff);
	char buffer[100];

	x_dest = x;
	y_dest = y;

	while(1)
	{
		counter=0;
		do	//Fill buffer with single word
		{
			if(TextString[i]==32 || TextString[i]=='\n' || TextString[i]=='\0')		//End of word markers
			{
				buffer[counter] = TextString[i];
				i++;
				break;
			}
			else
			{
				buffer[counter] = TextString[i];
				counter++;
				i++;
			}
		} while(1);

		wordwidth = (float)counter*(float)usefont->framewidth*screenratio;
		if(x_dest+wordwidth>=limit)		//If the word will take us past the limit given
		{
			y_dest += (int)((float)usefont->frameheight*screenratio);	//Start a new line
			x_dest = x;
		}

		//Blit word
		for(int j=0;j<=counter;j++)
		{
			if(buffer[j]==10)		//New line
			{
				y_dest += (int)((float)usefont->frameheight*screenratio);	//Start a new line
				x_dest = x;
			}
			else if(buffer[j]=='\0')
			{
				return 0;
			}
			else if(buffer[j]>31)
			{
				if(buffer[j]=='_') buffer[j] = 32;
				ch = buffer[j]-32;
				if(ch>=usefont->frames) ch = 0;
				DrawGraphicalObject(usefont, x_dest, y_dest, ch, 0, 1.8f*screenratio, true, eighthcolour);
				DrawGraphicalObject(usefont, x_dest, y_dest, ch, 0, 1.4f*screenratio, true, quarcolour);
				DrawGraphicalObject(usefont, x_dest, y_dest, ch, 0, screenratio, true, colour);
				x_dest += (int)((float)usefont->framewidth*screenratio);
			}
			else
			{
				x_dest += (int)((float)usefont->framewidth*screenratio);
			}
		}
	}

	return 0;
}

int DrawNumber(GraphicalObject *usefont, int number, int x, int y, int alpha)
{
	int units = 0, tens = 0, hundreds = 0, thousands = 0, tenthousands = 0;

	units = number%10;
	number -= units;
	tens = number%100;
	number -= tens;
	hundreds = number%1000;
	number -= hundreds;
	thousands = number%1000;
	number -= thousands;
	tenthousands = number/10000;
	thousands = number/1000;
	hundreds = hundreds/100;
	tens = tens/10;

	if(tenthousands)
	{
		DrawGraphicalObject(usefont, x, y, tenthousands, 0, alpha);
		x += usefont->framewidth;
	}
	if(thousands || tenthousands)
	{
		DrawGraphicalObject(usefont, x, y, thousands, 0, alpha);
		x += usefont->framewidth;
	}
	if(hundreds || thousands || tenthousands)
	{
		DrawGraphicalObject(usefont, x, y, hundreds, 0, alpha);
		x += usefont->framewidth;
	}
	if(tens || hundreds || thousands || tenthousands)
	{
		DrawGraphicalObject(usefont, x, y, tens, 0, alpha);
		x += usefont->framewidth;
	}
	DrawGraphicalObject(usefont, x, y, units, 0, alpha);
	return 0;
}

