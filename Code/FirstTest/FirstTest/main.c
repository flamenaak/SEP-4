/*
* FirstTest.c
*
* Created: 26/10/2016 13:55:41
* Author : IHA
*/

#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// FfreeRTOS Includes
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>

#include "src/board/board.h"

void update();
void matrixify();
void moveCar (uint16_t direction, uint16_t car[2]);
static const uint8_t _COM_RX_QUEUE_LENGTH = 30;
static QueueHandle_t _x_com_received_chars_queue = NULL;
static SemaphoreHandle_t  xMutex = NULL;

// frame_buf contains a bit pattern for each column in the display
//static uint16_t frame_buf[14] = {0,0,28,62,126,254,508,254,126,62,28,0,0,0};
static uint16_t frame_buf[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint16_t myMatrix[14][10];
uint16_t car[2];

//-----------------------------------------
void another_task(void *pvParameters)
{
	// The parameters are not used
	( void ) pvParameters;

	#if (configUSE_APPLICATION_TASK_TAG == 1)
	// Set task no to be used for tracing with R2R-Network
	vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );
	#endif
	
	BaseType_t result = 0;
	uint8_t byte;
		
		for(int i = 0; i < 14; i++){
			for(int j = 0; j < 10; j++){
				myMatrix[i][j] = 0;
			}
		}
	car[0] = 6; //column
	car[1] = 9; // row
	myMatrix[car[0]][car[1]] = 1;
	update();
	while(1)
	{	
		//result = xQueueReceive(_x_com_received_chars_queue, &byte, 1000L);
			
    if((~PINC & (1<<PINC0)) != 0){
			moveCar(0, car); //down
		}

	if((~PINC & (1<<PINC1)) != 0){
		moveCar(2, car);// right
	}
	if((~PINC & (1<<PINC6)) != 0){
		moveCar(1, car); //up
	}
	if((~PINC & (1<<PINC7)) != 0){
		moveCar(3,car); //left
	}
		
		///if (result) {
//			com_send_bytes(&byte, 1);
	//	}else {
	//		com_send_bytes((uint8_t*)"TO", 2);
	//	}
	}
}

void moveCar (uint16_t direction, uint16_t car[2]){
	
	switch(direction){
	case 0: 	
			if((car[1] + 1 <= 9) && myMatrix[car[0]][car[1] + 1] == 0){
				myMatrix[car[0]][car[1]] = 0;
				myMatrix[car[0]][++car[1]] = 1;
				update();
				vTaskDelay(200);}
    break;
	
	
	case 1:
			if(car[1] >= 1 && myMatrix[car[0]][car[1] - 1] == 0){
				myMatrix[car[0]][car[1]] = 0;
				--car[1];
				myMatrix[car[0]][car[1]] = 1;
				update();
				vTaskDelay(200);
			}
	
	break;
	case 2:
					if((car[0] + 1 <= 13) && myMatrix[car[0] + 1][car[1]] == 0){
					myMatrix[car[0]][car[1]] = 0;
					myMatrix[++car[0]][car[1]] = 1;
					update();
					vTaskDelay(200);
				}
	
	break;
	case 3:
					if((car[0] >= 1) && myMatrix[car[0] - 1][car[1]] == 0){
					myMatrix[car[0]][car[1]] = 0;
					myMatrix[--car[0]][car[1]] = 1;
					update();
					vTaskDelay(200);
					}
	
	break;
	}
}

//  void matrixify(){
// 	for(int i = 0; i < 14; i++){
// 		uint16_t num = frame_buf[i];
// 		for(int j = 9; j >= 0; j--){
// 			if(num >= (2^j)){
// 				myMatrix[i][j] = 1;
// 				num -= (2^j);
// 			}
// 			else
// 			myMatrix[i][j] = 0;
// 		}
// 	}
// }

void update(){
			for (int i = 0; i < 14; i++)
			{
				frame_buf[i] = 0;
				for(int j =0; j < 10 ; j++){
					if(myMatrix[i][j] == 1){
						 frame_buf[i] |= 1<<j;
					}
					else {
						
					}				
				}
			}
}
//-----------------------------------------
 static uint8_t obstacles[14][10] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
										 {0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
//-----------------------------------------
void obstacles_task(void *pvParameters){



					

					while(1){
					
// 					for(int j = 0; j < 10; j++){
// 						for(int i = 0; i < 14; i++){
// 							if(i != car[0] && j != car[1]){
// 								myMatrix[i][1+j] = myMatrix[i][j];
// 								if(j==0)
// 								{
// 									myMatrix[i][j] = 0;
// 								}
// 								
// 							}
// 						}
										
										if(obstacles[car[0]][car[1]-1] == 0){
											int count = 0;
											uint8_t aux[14][10];
											for(int i = 0; i < 14; i++){
												for(int j = 0; j < 9; j++){
													// 						if(i != car[0] && j != car[1]){
													// 							myMatrix[i][j] = obstacles[i][j];
													// 							}
													myMatrix[i][1+j] = obstacles[i][j];
												}
											}
											for (int i = 1; i < 14; i++)
											{
												myMatrix[0][0] = rand()%2;
												if(count < 2 || i == 13) {
												myMatrix[i][0] = rand()%2;
													if(myMatrix[i][0] == 1)
													count++;
													if(i % 4 == 0) count = 0;
												}
												else {
												myMatrix[i][0] = 0;
												}
												 //column - row
											}
											for(int i = 0; i < 14; i++){
												for(int j = 0; j < 9; j++){
													obstacles[i][j] = myMatrix[i][j];
												}
											}
											myMatrix[car[0]][car[1]] = 1;
	
	
										}

											update();
											vTaskDelay(1600);
					}
						
					

}
//-----------------------------------------
void startup_task(void *pvParameters)
{
	// The parameters are not used
	( void ) pvParameters;

	#if (configUSE_APPLICATION_TASK_TAG == 1)
	// Set task no to be used for tracing with R2R-Network
	vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
	#endif
	
	_x_com_received_chars_queue = xQueueCreate( _COM_RX_QUEUE_LENGTH, ( unsigned portBASE_TYPE ) sizeof( uint8_t ) );
	init_com(_x_com_received_chars_queue);
	
	// Initialise Mutex
	xMutex = xSemaphoreCreateMutex();
	
	// Initialization of tasks etc. can be done here
	BaseType_t t1 = xTaskCreate(another_task, (const char *)"Another", configMINIMAL_STACK_SIZE, (void *)NULL, 5, NULL);
	BaseType_t t2 = xTaskCreate(obstacles_task, (const char *)"Obstacles", configMINIMAL_STACK_SIZE, (void *)NULL, 4, NULL);
	
	
	
	// Lets send a start message to the console
	com_send_bytes((uint8_t *)"Then we Start!\n", 15);
	
	while(1)
	{
	
	}
}

// Prepare shift register setting SER = 1
void prepare_shiftregister()
{
	// Set SER to 1
	PORTD |= _BV(PORTD2);
}

// clock shift-register
void clock_shift_register_and_prepare_for_next_col()
{
	// one SCK pulse
	PORTD |= _BV(PORTD5);
	PORTD &= ~_BV(PORTD5);
	
	// one RCK pulse
	PORTD |= _BV(PORTD4);
	PORTD &= ~_BV(PORTD4);
	
	// Set SER to 0 - for next column
	PORTD &= ~_BV(PORTD2);
}

// Load column value for column to show
void load_col_value(uint16_t col_value)
{
	PORTA = ~(col_value & 0xFF);
	
	// Manipulate only with PB0 and PB1
	PORTB |= 0x03;
	PORTB &= ~((col_value >> 8) & 0x03);
}

//-----------------------------------------
void handle_display(void)
{
	static uint8_t col = 0;
	
	if (col == 0)
	{
		prepare_shiftregister();
	}
	
	load_col_value(frame_buf[col]);
	
	clock_shift_register_and_prepare_for_next_col();
	
	// count column up - prepare for next
	col++;
	if (col > 13)
	{
		col = 0;
	}
}

//-----------------------------------------
void vApplicationIdleHook( void )
{
	//
}

//-----------------------------------------
int main(void)
{
	
	init_board();
	
	// Shift register Enable output (G=0)
	PORTD &= ~_BV(PORTD6);
	
	//Create task to blink gpio
	BaseType_t t1 = xTaskCreate(startup_task, (const char *)"Startup", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY, NULL);

	// Start the display handler timer
	init_display_timer(handle_display);
	
	sei();
	
	//Start the scheduler
	vTaskStartScheduler();
	
	//Should never reach here
	while (1)
	{
	}
}


