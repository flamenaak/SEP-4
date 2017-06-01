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
static xQueueHandle _x_com_received_chars_queue = NULL;
static SemaphoreHandle_t  xMutex = NULL;

static uint8_t *current_message;
static uint8_t ackFLAG = 0;
static xQueueHandle xSendQueue;
static xQueueHandle xACKQueue;
static xQueueHandle xInputQueue;


/************************************************************************/
/* System Timer for SENDER TIMEOUT                                      */
/************************************************************************/
void vTimeout(TimerHandle_t xTimer);
void vTimeout(TimerHandle_t xTimer) {
	com_send_bytes((*current_message), 2);
}
static TimerHandle_t sender_timeout;

//***********************************************************************
//***********************************************************************


// frame_buf contains a bit pattern for each column in the display
//static uint16_t frame_buf[14] = {0,0,28,62,126,254,508,254,126,62,28,0,0,0};
static uint16_t frame_buf[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//-----------------------------------------
static uint8_t obstacles[14][10];
										
										
static uint16_t myMatrix[14][10]; //guarded by mutex
static uint16_t car[2];
static uint16_t car1[2];
static int stop = 1;
static int restart1 = 0;
static int restart2 = 1;

//----------------------------------------
struct input{
	uint16_t direction;
	uint16_t car[2];
}Input;

struct input1{
	uint16_t direction;
	uint16_t car1[2];
};

struct car{
	uint16_t x,y;
	uint8_t computer;
} Car;


void moveCar (uint16_t direction, uint16_t car[2]){
	
	switch(direction){
		case 0:
		if((car[1] + 1 <= 9) && myMatrix[car[0]][car[1] + 1] == 0){
			myMatrix[car[0]][car[1]] = 0;
			myMatrix[car[0]][++car[1]] = 1;
		break;
		}
		
		case 1:
		if(car[1] >= 1 && myMatrix[car[0]][car[1] - 1] == 0){
			myMatrix[car[0]][car[1]] = 0;
			--car[1];
			myMatrix[car[0]][car[1]] = 1;
			}
		
		break;
		case 2:
		if((car[0] + 1 <= 13) && myMatrix[car[0] + 1][car[1]] == 0){
			myMatrix[car[0]][car[1]] = 0;
			myMatrix[++car[0]][car[1]] = 1;
		}
		
		break;
		case 3:
		if((car[0] >= 1) && myMatrix[car[0] - 1][car[1]] == 0){
			myMatrix[car[0]][car[1]] = 0;
			myMatrix[--car[0]][car[1]] = 1;
		}
		
		break;
	}
}

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
void obstacles_task(void *pvParameters){
	#if (configUSE_APPLICATION_TASK_TAG == 1)
			vTaskSetApplicationTaskTag( NULL, ( void * ) 2 ); // Set task no to be used for tracing with R2R-Network
	#endif
	
	while(1){		
						if(stop == 0){
							if(obstacles[car[0]][car[1]-1] == 0 && obstacles[car1[0]][car1[1]-1] == 0){
								if(xSemaphoreTake(xMutex, portMAX_DELAY)){
											int count = 0;
											for(int i = 0; i < 14; i++){
												for(int j = 0; j < 9; j++){
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
					
											}
											for(int i = 0; i < 14; i++){
												for(int j = 0; j < 9; j++){
													obstacles[i][j] = myMatrix[i][j];
												}
											}
											myMatrix[car[0]][car[1]] = 1;
											myMatrix[car1[0]][car1[1]] = 1;
											xSemaphoreGive(xMutex);
										}
										
									}
									else{
										stop = 1;
									}
						}
						vTaskDelay(1000);
	}
}

//-----------------------------------------
void displayUpdater_task(void *pvParameters)
{
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	// Set task no to be used for tracing with R2R-Network
	vTaskSetApplicationTaskTag( NULL, ( void * ) 3 );
	#endif
	while(1)
	{
				update();
				
		
		vTaskDelay(41);
	}
}
//-----------------------------------------
void gameLogic_task(void *pvParameters)
{
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	// Set task no to be used for tracing with R2R-Network
	vTaskSetApplicationTaskTag( NULL, ( void * ) 4 );
	#endif
	
	while(1){
				if(stop == 0){
					struct input inp;
							
							if(xQueueReceive(xInputQueue, (void*)&inp, 2000)){
									if(xSemaphoreTake(xMutex, portMAX_DELAY)){
											if(inp.car[0] == car[0] && inp.car[1] == car[1]){
												moveCar(inp.direction, car);
											}
											else if(inp.car[0] == car1[0] && inp.car[1] == car1[1]){
												moveCar(inp.direction, car1);
											}
								
									xSemaphoreGive(xMutex);
									}
							}				
				}
				else{
					if(restart1 + restart2 + stop == 2){
						stop = 0;
						restart1 = 0;
						restart2 = 1; //later 0 when we add a player
						setupGame();
					}
				vTaskDelay(50);			
				}
				
			
	}
}
//-----------------------------------------
void joystickSampler_task(void *pvParameters)
{
	#if (configUSE_APPLICATION_TASK_TAG == 1)
			vTaskSetApplicationTaskTag( NULL, ( void * ) 5 ); // Set task no to be used for tracing with R2R-Network
	#endif
	struct input inp;
	
	while(1)
	{
		if(stop == 0)
		{
			inp.car[0] = car[0];
			inp.car[1] = car[1];
			if((~PINC & (1<<PINC0)) != 0){
				inp.direction = 0;
				//moveCar(0,car);
				xQueueSend(xInputQueue, (void*)&inp , portMAX_DELAY); //down
			}

			if((~PINC & (1<<PINC1)) != 0){
				inp.direction = 2;
				//moveCar(2,car);
				xQueueSend(xInputQueue, (void*)&inp, portMAX_DELAY);// right
			}
			if((~PINC & (1<<PINC6)) != 0){
				inp.direction = 1;
				//moveCar(1,car);
				xQueueSend(xInputQueue, (void*)&inp, portMAX_DELAY); //up
			}
			if((~PINC & (1<<PINC7)) != 0){
				inp.direction = 3;
				//moveCar(3,car);
				xQueueSend(xInputQueue, (void*)&inp, portMAX_DELAY); //left
			}
		}
		else{
			if((~PIND & (1<<PIND3)) != 0){
				restart1 = 1;
			}
		}
		vTaskDelay(100);
	}
}

void setupGame(){
	for(int i = 0; i < 14; i++) {		//Setup matrix
		for(int j = 0; j < 10; j++){
			myMatrix[i][j] = 0;
			obstacles[i][j] = 0;
		}
	}
	car[0] = 5; //column				//Start position for the first car
	car[1] = 9; // row

	car1[0] = 9; //column				//Start position for the second car
	car1[1] = 7; // row

	myMatrix[car[0]][car[1]] = 1;		//Placement of the car on the matrix
	myMatrix[car1[0]][car1[1]] = 1;		//Placement of the car on the matrix
	//update();
}

// -----------------------------------------
void comSender_task(void *pvParameters)
{
	
			//format:::: header of 2 bits + message of 3 bits = 5 bits of payload -> 4 parity bits of hamming code // 9 bits to send
			//01 xxx - simple message
			//10 xxx - ACK
			//11 xxx - NACK
			//need global var for tracking the current message - so no new item will be dequeued if no ACK was received
			// have to make sure that it doesnt send a MESSAGE from CS before an ACK from the receiver: idea: maybe having a flag for sending ACK or NACK before anything else in the sender
			while (1) {
				if (ackFLAG == 0) { //is not waiting for ACK
					if (xQueueReceive(xSendQueue, &(current_message), (TickType_t)10)) {
						com_send_bytes((*current_message), 2); //maybe &
						ackFLAG = 1;
						xTimerStart(sender_timeout, pdMS_TO_TICKS(10));
					}
				}
				else { // waiting for ACK
					uint8_t *ACK = NULL;
					xQueueReceive(xACKQueue, &(ACK), (TickType_t)10);
					if (ACK) com_send_bytes((*ACK), 2); // maybe &
				}

				vTaskDelay(10);

			}
		
}
//-----------------------------------------
void comReceiver_task(void *pvParameters)
{
	
		while (1) {
			BaseType_t result = 0;
			BaseType_t payload = 0;
			uint8_t byte;
			result = xQueueReceive(_x_com_received_chars_queue, &byte, 1000L);//receives 9 bits
			if (result) {
				//payload = check_parity_bits(*byte);
				//01 xxx - simple message
				//10 xxx - ACK
				//11 xxx - NACK
				uint8_t type = result & ((1 << 6) | (1 << 7));
				uint8_t sequence = result & (1 << 5);

				if (type == ((1 << 6))) //message
				{

					uint8_t ack = ((1 << 7) | sequence);
					uint8_t cmd = 0;

					for (uint8_t i = 0; i < 3; i++)
					{
						if (result && (1 << i)) cmd += 1 << i;
					}
					xQueueSend(xACKQueue, &ack, 1000L);  //send ack
					xQueueSend(xInputQueue, &cmd, 1000L);
				}
				if (type == (1 << 7)) //ACK
				{
					//nice, do nothing, or maybe unreference the current message, because there is no need to resend
					ackFLAG = 0;
					xTimerStop(sender_timeout, pdMS_TO_TICKS(10));
				}
				if (type == ((1 << 7) | (1 << 6))) //NACK
				{

					//retransmit message - aka do nothing, cause there is a timeout
				}
			}

			vTaskDelay(10);
		}
}



//-----------------------------------------
/***
*	Start UP Task
*
***/
void startup_task(void *pvParameters)
{
	( void ) pvParameters;  // The parameters are not used

	#if (configUSE_APPLICATION_TASK_TAG == 1)
			vTaskSetApplicationTaskTag( NULL, ( void * ) 1 ); // Set task no to be used for tracing with R2R-Network
	#endif
	
	setupGame();
	
	xMutex = xSemaphoreCreateMutex();  // Initialise Mutex
		
	BaseType_t tDU = xTaskCreate(displayUpdater_task, (const char *)"Display updater", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 2, NULL);
	BaseType_t tGL = xTaskCreate(gameLogic_task, (const char *)"Game logic", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 7, NULL);
	BaseType_t t2 = xTaskCreate(obstacles_task, (const char *)"Obstacles", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 6, NULL);
	BaseType_t tJS = xTaskCreate(joystickSampler_task, (const char *)"Joystick sampler", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 3, NULL);
	
	BaseType_t tCS1 = xTaskCreate(comSender_task, (const char *)"Communication sender", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 5, NULL);
	BaseType_t tCR1 = xTaskCreate(comReceiver_task, (const char *)"Communication receiver", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY + 4, NULL);
	//BaseType_t tOBS = xTaskCreate(obstacles_task, (const char *)"Obstacles", configMINIMAL_STACK_SIZE, (void *)NULL, 1, NULL);
	
	
	while(1);
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



//----------------------------------------- MAIN ---------------------------------------------------------------
int main(void)
{
	init_board();
	_x_com_received_chars_queue = xQueueCreate( _COM_RX_QUEUE_LENGTH, ( unsigned portBASE_TYPE ) sizeof( uint8_t ) );
	//init_com(_x_com_received_chars_queue);
	//sender_timeout = xTimerCreate("Timeout", pdMS_TO_TICKS(100), pdFALSE, 1, vTimeout);

	xSendQueue = xQueueCreate(5, sizeof(unsigned char));
	xACKQueue = xQueueCreate(5, sizeof(unsigned char));
	xInputQueue = xQueueCreate(10, sizeof(struct input));

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


