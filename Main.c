/*
Name: Vincent McLoughlin 	User ID: 20575319
Name: David Weisdorf 			User ID: 20553320
*/

#include <RTL.h>
#include "LPC17xx.H"                    /* LPC17xx definitions               */
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "ADC.h"
#include "math.h"
#include "drawing.h"
#include "endScreen.c"
#include "victoryScreen.c"



#define __FI        1                   /* Font index 16x24                  */

OS_TID t_led;                           /* assigned task id of task: led */
OS_TID t_adc;                           /* assigned task id of task: adc */
OS_TID t_kbd;                           /* assigned task id of task: keyread */
OS_TID t_model   ;                        /* assigned task id of task: joystick */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_draw;                           /* assigned task id of task: lcd     */

OS_SEM updateModel;
OS_SEM drawNow;

unsigned int ADCStat = 0;
unsigned int ADCValue = 0; //ADC value for the potentiometer is 0-2^12-1, which is 0 - 4095

#define MAP_SIZE 2560
#define PIXELS_PER_ITERATION 4
#define OBSTACLE_WIDTH 20
#define PLAYER_WIDTH 40
#define SCREENWIDTH 320

/* Game Data */
/* General */
unsigned short int isButtonPressed = 0;

unsigned char globalMap[MAP_SIZE];
unsigned char mapComponents[MAP_SIZE / OBSTACLE_WIDTH] = {	// o represents a gap 20 pixels wide, 1 represents an object of dimensions OBSTACLE_WIDTH by OBSTACLE_WIDTH																											\
			/* First Screen all Zeros */																				
			0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, /**/ 0, 0, 0, 0, 			
																																					
			/* Each Full line is a screen width */
			1, 0, 0, 0, /**/ 0, 1, 0, 0, /**/ 0, 0, 1, 1, /**/ 0, 0, 0, 0,
			0, 0, 1, 0, /**/ 0, 0, 0, 1, /**/ 1, 0, 0, 0, /**/ 1, 1, 0, 0,
			0, 1, 1, 0, /**/ 0, 0, 1, 0, /**/ 0, 0, 0, 1, /**/ 0, 0, 0, 0,
			0, 0, 0, 1, /**/ 0, 0, 0, 0, /**/ 0, 1, 1, 0, /**/ 0, 0, 1, 0,
			0, 0, 1, 0, /**/ 0, 0, 0, 1, /**/ 0, 0, 0, 0, /**/ 0, 0, 1, 1,
			0, 0, 0, 1, /**/ 0, 0, 1, 1,
			
			// last 360 are 0s
			0, 0, 0, 0, 0, 0, /**/ 0, 0, 0, 0, 0, 0, /**/ 0, 0, 0, 0, 0, 0
};

unsigned short int numLives = 3;
double frameDelay = 10; //Delay between each frame. Lower value results in less delay between frames.

/* Coordinates */
/*
		x: x is the same in all coordinate systems
		y: there are three coordinate systems
			1. Static LCD coordinate system. Goes from 0 - 319, where 0 is leftmost, 319 is rightmost
			2. Dynamic LCD coordinate system. Every time you scroll the LCD, it shifts the coordinate system
					goes 0 - 319, but you have to keep track of which is left most and which is rightmost
			3. Map coordinate system. Player moves through a map so to speak. Goes 0 - Map size
*/


int map_playerX = 0;
//2. dynamic coords
unsigned short lcd_dynamic_leftMost = 0;
unsigned short lcd_dynamic_rightMost = 319;
unsigned short lcd_dynamic_prevRightMost = 319 - PIXELS_PER_ITERATION;

//3. Map coord
unsigned short map_playerY = 0;
int verticalVel = 0;

void shiftScreen(unsigned int dy) // Shifts screen while keeping track of the LCD coordinate system
{
	GLCD_ScrollVertical(dy);
	lcd_dynamic_prevRightMost = lcd_dynamic_rightMost;
	lcd_dynamic_rightMost = (lcd_dynamic_rightMost + dy) % SCREENWIDTH;
	lcd_dynamic_leftMost = (lcd_dynamic_leftMost + dy) % SCREENWIDTH;
}

void endGame() 
{
		os_tsk_delete (t_adc);
		os_tsk_delete (t_draw);
	
		while(lcd_dynamic_leftMost != 0)
			shiftScreen(1);
	
		if(numLives == 0){
			GLCD_SetBackColor(Black);
			GLCD_SetTextColor(Red);
			GLCD_Clear(Black);
			GLCD_DisplayString  (2,  5, __FI, "Game Over");
	  //Write Game Over, stop all other drawing.
		}
		else
		{
			unsigned int backColor = Magenta;
			GLCD_SetBackColor(backColor);
			GLCD_SetTextColor(Yellow);
			GLCD_Clear(backColor);
			GLCD_DisplayString  (2,  5, __FI, "You Win!!");	
		//Write Victory Screen, stop all other drawing.
		}
		while(1) {;}
		
}

void buildMaps(void) 
{
	int i, j;
		
	//each value in map components represents the presence or absence of a obstacle
	// just a compressed format for globalMap

		
		for(i = 0; i < MAP_SIZE / OBSTACLE_WIDTH; i++) 
		{
				for(j = 0; j < OBSTACLE_WIDTH; j++) 
				{
					globalMap[i*OBSTACLE_WIDTH + j] = mapComponents[i];
				}
		}
}

void updateNumLivesLED(){
	
	if(numLives == 3){
		
		LED_On(7);
		LED_On(6);
		LED_On(5);
		
	}
	else if(numLives == 2){
		
		LED_Off(5);
		
	}
	else if(numLives == 1){
		
		LED_Off(6);
		
	}
	else //Player is has 0 lives, display game over screen
	{
		LED_Off(7);
	}		
}

/*----------------------------------------------------------------------------
  Task 3 'ADC': Read potentiometer
 *---------------------------------------------------------------------------*/
/*NOTE: You need to complete this function*/
__task void adc (void) {

  while(1) {
		  ADC_ConversionStart();
			// Get the value from ADC	
		
		frameDelay = 2.5057E-6 *ADCValue*ADCValue + (0.000737253)*ADCValue +1;
		//Maps the ADC Value to a quadratic scale, where 1 will yield the fastest speed (ADC = 0) and 40 is the slowest speed (ADC = 4095)
		
	}
}

int checkCollision(){
	
	int collisionFlag = 0;
	int i = 0;
	
	if(map_playerX > OBSTACLE_WIDTH ){ //If Player is above the obstacle's height, no collision occurs
		return collisionFlag;
	}
	
	for( i = 0; i < PLAYER_WIDTH; i++){
	
		if(globalMap[map_playerY + i] == 1){ //Checks if there is an obstacle colliding with the player
		collisionFlag = 1;
		}
	}
	
	return collisionFlag;
}

__task void modelUpdate (void) {
	
	int collisionFlag;
	
  while(1) {
		os_sem_wait(&updateModel, 0xFFFF);  //Waiting for the previous model to indicate it's done.
		os_tmr_create(frameDelay, 1); //Setting up the timer that will trigger the next draw after this model update
		
		//update block height
		if(isButtonPressed && (map_playerX == 0))
		{
			verticalVel = 12;
		}
		
		map_playerX = map_playerX + verticalVel;
		
		if(map_playerX < 0) 
			map_playerX = 0;
		
		if(map_playerX == 0)
			verticalVel = 0; 
		else
			verticalVel = verticalVel - 1;                                                                                                                                                                                                                                                                            
		
		map_playerY += PIXELS_PER_ITERATION; // Shifts player's horizontal position by a constant value every cycle
		
		os_sem_send(&drawNow);
		
		/*	Check for collision */
		collisionFlag = checkCollision();
		
		if(collisionFlag == 1){ // If collision occurs, decrease number of lives and indicate on LEDs.
			
			int i;
			
			numLives -= 1;
			updateNumLivesLED();
			if(numLives == 0) 
				endGame(); // Give game over if out of lives
			
			i = 0;
			while(globalMap[map_playerY + i] == 0) //Finds the beginning of the obstacle
				i++;
			
			while(globalMap[map_playerY + i + 1] == 1) //Deletes each column of pixels for the obstacle.
			{
				globalMap[map_playerY + i] = 0;
				i++;
			}
			globalMap[map_playerY + i] = 0; // Deletes the last column. The while loop stops before the last.
		}
		
		/* Check if we are at the end */
		if(map_playerY + 320 > MAP_SIZE)	// If we are at the end break out of the while loop and go straight to end game screen
			break;
	}
	
	// Display game win
	endGame();
}

__task void drawPeriodic (void) {
	unsigned int i = 1, j;
	
  while(1) {
		os_sem_wait(&drawNow, 0xffff);
		
		shiftScreen(PIXELS_PER_ITERATION);
		drawCube(map_playerX, lcd_dynamic_leftMost, -1, -1);

		//draw an obstacle if one exists
		for(j = 0; j < PIXELS_PER_ITERATION; j++) 
		{
			if(globalMap[map_playerY + 319 - PIXELS_PER_ITERATION + j] == 1)
			{
				GLCD_ClearRegion (Black, 0, (lcd_dynamic_prevRightMost + j) % 320, OBSTACLE_WIDTH - 1, 1);
			} 
			else 
			{
				GLCD_ClearRegion (White, 0, (lcd_dynamic_prevRightMost + j) % 320, OBSTACLE_WIDTH - 1, 1);
			}
		}		

		i++;
	}
}


/*----------------------------------------------------------------------------
  Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
/* NOTE: Add additional initialization calls for your tasks here */

__task void init (void) {

	os_sem_init(&updateModel, 0);
	os_sem_init(&drawNow, 0);
	buildMaps();
	updateNumLivesLED();
	
	t_model = os_tsk_create(modelUpdate, 9);
	t_draw = os_tsk_create (drawPeriodic, 10);     /* start task lcd                   */
	t_adc = os_tsk_create(adc, 1);
	
	os_tmr_create (frameDelay, 1); 
	
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	
	NVIC_EnableIRQ( ADC_IRQn ); 							/* Enable ADC interrupt handler  */					
	NVIC_EnableIRQ( EINT3_IRQn );							/* Enable Push Button interrupt handler  */

  LED_Init ();                              /* Initialize the LEDs           */
  GLCD_Init();                              /* Initialize the GLCD           */
	KBD_Init ();                              /* initialize Push Button        */
	ADC_Init ();															/* initialize the ADC            */

  GLCD_Clear(White);                        /* Clear the GLCD                */
	
  os_sys_init(init);                        /* Initialize RTX and start init */
}
