#include "drawing.h"
#include "GLCD.h"

int prevX = -1;
int prevY = -1;

#define CUBELENGTH 40
#define NUMPIXELSINCUBE (CUBELENGTH*CUBELENGTH)
#define BITMAPSIZE (NUMPIXELSINCUBE*2)

unsigned char cubeMap[BITMAPSIZE];

void drawCube(int x, int y, int w, int h) //Draws red cube of dimenstions CUBELENGTH*CUBELENGTH
{
		static short hasInitialized = 0;
		if(!hasInitialized) // Creates a bitmap 
		{
			int i, byte1, byte2;
			hasInitialized = 1;
			for(i = 0; i < (BITMAPSIZE/2); i++) 
			{
				//argb format. Every 2chars is one pixel
				byte1 = 2*i;
				byte2 = byte1+1;
				cubeMap[byte1] = 0x00;
				cubeMap[byte2] = 0xF8;
			}
		}
		
		//Check if we need to erase cube at previous position. If so, we erase it.
		if(prevX >= 0 && prevY >= 0)
		{
					GLCD_ClearRegion(White, prevX, prevY, CUBELENGTH, CUBELENGTH);
					if(prevY > 280)
						GLCD_ClearRegion(White, prevX, 0, CUBELENGTH, CUBELENGTH - (319 - prevY));
		}

		GLCD_Bitmap (x, y, CUBELENGTH, CUBELENGTH, &(cubeMap[0]));
		
		//we may have tried to draw "off screen" so to speak, due to scrolling effect
		// so we have to draw at front of screen to compensate
		if(y > 280) 
				GLCD_Bitmap (x, 0, CUBELENGTH, CUBELENGTH - (319 - y), &(cubeMap[0]));	
		
		prevX = x;
		prevY = y;
		
}

void drawSquareObstacle(void) 
{
	GLCD_ClearRegion(Black, 1, 1, 40, 40);
}
