/*
 * TP4 OSII - FreeRTOS
 *
 * @author Casabella Martin *
 *
 *
 * */




#include <string.h>
#include <math.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Priorities at which the tasks are created. */
#define RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	PERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	APERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
/* The bit of port 0 that the LPCXpresso LPC13xx LED is connected. */
#define mainLED_BIT 						( 22 )
#define MAX_POINTERS		10
#define ioMAX_MSG_LEN	( 50 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define PERIODIC_SEND_FREQUENCY_MS			( 50 / portTICK_RATE_MS )

#define MAX_STRING_LEN	51
#define BUF_TEMP 8
#define portNEW_DELAY 200


#if (defined(__NEWLIB__))
#define LIBSTUB_SYS_WRITE _swiwrite
#else // __REDLIB__
#define LIBSTUB_SYS_WRITE __write
#endif


/*Prototipo de funciones*/
static void vReceiverTask(void *pvParameters);
static void vPeriodicSenderTask(void *pvParameters);
static void vAperiodicSenderTask(void *pvParameters);

void vPrintString( const portCHAR *pcString );
void vPrintStringAndNumber( const portCHAR *pcString, unsigned portLONG ulValue );
int consoleprint(char *cpstring);
int LIBSTUB_SYS_WRITE (int, char *, int);

char * vRandomString(int len_string);

/* Simple function to toggle the LED on the LPCXpresso LPC17xx board. */
static void vToggleLED(void);

static char cBuffer[ ioMAX_MSG_LEN ];




uint32_t coreClock_1 = 0; /* Variables to store core clock values */
uint32_t coreClock_2 = 0;

/* Declare a variable of type QueueHandle_t to hold the handle of the queue being created. */
QueueHandle_t xPointerQueue;

struct msg_struct {
	char * msg;
	char num;
	int len_str;
};

int main(void) {
	/* Initialise P0_22 for the LED. */
	LPC_PINCON->PINSEL1 &= (~(3 << 12));
	LPC_GPIO0->FIODIR |= (1 << mainLED_BIT);
	coreClock_1 = SystemCoreClock; /* Store value of predefined SystemCoreClock */
	SystemCoreClockUpdate(); /* Update SystemCoreClock according to register settings */
	coreClock_2 = SystemCoreClock; /* Store value of calculated SystemCoreClock */
	if (coreClock_2 != coreClock_1) { /* Without changing the clock setting both core clock values should be the same */
		// Error Handling
	}

	printf("\nLPC System Clock: %d[Mhz]\n\n", coreClock_2 / (1000000));

	/* Enable traceanalycer snapshot */
	vTraceEnable(TRC_START);

	/* Create a queue that can hold a maximum of MAX_POINTERS pointers*/
	xPointerQueue = xQueueCreate(MAX_POINTERS, sizeof(struct msg_struct *)); //creo cola de 10 punteros a struct

	if (xPointerQueue != NULL) {
		/* Start the two tasks as described in the accompanying application
		 note. */
		//For example, the Cortex-M3 stack is 32 bits wide so, if
		//usStackDepth is passed in as 100, then 400 bytes of stack space will be allocated (100 * 4 bytes).
		/*
		 * The size of the stack used by the idle task is defined by the applicationdefined constant configMINIMAL_STACK_SIZE . The value assigned to
		 * this constant in the standard FreeRTOS Cortex-M3 demo applications is the minimum recommended for any task. If your task uses a lot of stack
		 * space, then you must assign a larger value.		 *
		 *
		 *
		 */



		if (xTaskCreate(vReceiverTask, "Receiver", configMINIMAL_STACK_SIZE,
				NULL, configMAX_PRIORITIES - 2, NULL) == pdTRUE) {
			printf("Receiver task created\n");
		} else {

			printf("Could not create Receiver task \n");
		};

		if (xTaskCreate(vPeriodicSenderTask, "TemperatureSensor",
				configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
				NULL) ==pdTRUE) {

			printf("Periodic Sender task created\n");
		} else {

			printf("Could not create Periodic Sender task \n");
		};

		if (xTaskCreate(vAperiodicSenderTask, "User", configMINIMAL_STACK_SIZE,
				NULL, configMAX_PRIORITIES - 1, NULL) == pdTRUE) {

			printf("Aperiodic Sender task created\n\n\n");

		} else {

			printf("Could not create Aperiodic Sender task \n");

		};

		//taskcode, name, stacksize, parameters, handletask

		/**
		 * Passing a uxPriority value above (configMAX_PRIORITIES – 1) will result in the priority assigned to the task being capped silently to the
		 * maximum legitimate value.
		 *
		 */

		vTaskStartScheduler();

	}

	/* If all is well we will never reach here as the scheduler will now be
	 running.  If we do reach here then it is likely that there was insufficient
	 heap available for the idle task to be created. */
	for (;;)
		;
	return 0;
}

/**
 * Tarea de envio periodico representando el seensor de temperatura. Se genera
 * un valor entre 0 y 255, emulando medicion, y esta tarea se encarga de comunicarla
 * con el proceso central.
 *
 *
 * @param void*  pvParameters
 */
static void vPeriodicSenderTask(void *pvParameters) {

	/*	This parameter is named on the assumption that vTaskDelayUntil() is
	 being used to implement a task that executes periodically and with a
	 fixed frequency. */

	/* The xLastWakeTime variable needs to be initialized with the current tick
	 count. Note that this is the only time the variable is written to explicitly.
	 After this xLastWakeTime is updated automatically internally within
	 vTaskDelayUntil(). */
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	/* Define a task that performs an action every 50 milliseconds. */
	const TickType_t xPeriod = pdMS_TO_TICKS(150);

	struct msg_struct *msg = pvPortMalloc(sizeof(struct msg_struct *));

	/* Enter the loop that defines the task behavior. */
	for (;;) {

		/* This task should execute every 50 milliseconds. Time is measured
		 in ticks. The pdMS_TO_TICKS macro is used to convert milliseconds
		 into ticks. xLastWakeTime is automatically updated within vTaskDelayUntil()
		 so is not explicitly updated by the task. */

		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time.

		 The parameters to vTaskDelayUntil() specify, instead, the exact tick count value at which the
		 calling task should be moved from the Blocked state into the Ready state. vTaskDelayUntil()
		 is the API function that should be used when a fixed execution period is required (where you
		 want your task to execute periodically with a fixed frequency), as the time at which the calling
		 task is unblocked is absolute, rather than relative to when the function was called (as is the
		 case with vTaskDelay()).

		 */
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		msg->msg = NULL;
		msg->num = (char) (rand() % 255);

		/* Send to the queue - causing the queue receive task to flash its LED.
		 *
		 * 	0 is used as the block time so the sending operation will not block -
		 * 	it shouldn't need to block as the queue should always be empty at this
		 * 	point in the code.
		 *
		 * 	*/

		/*(pdMS_TO_TICKS(5): The maximum amount of time the task should remain in the
		 *Blocked state to wait for space to become available on the queue,
		 *should the queue already be full.*/
		if (xQueueSend( xPointerQueue, &msg, pdMS_TO_TICKS(10) ) != pdTRUE) {
			printf("\nFailed to post the message\n [P--PERIODIC TASK]\n\n");
		};		//espera 5 [ms] si la cola esta llena

		/*Free a estructura*/
		vPortFree(msg);

	}
}

/**
 * Funcion Aperiodica representa el ingreso de caracteres de un usuario. Se genera una cadena de longitud
 * variable, de manera aleatoria, y se envia aperiodicamente (i,e cada cierto tiempo que tambien es variable)
 *
 * @param: void*  pvParameters
 *
 */
static void vAperiodicSenderTask(void *pvParameters) {

	portTickType xLastWakeTime;
	/*Aloco estructura donde colocara cada string generado para colocar en la cola*/
	struct msg_struct *msg = pvPortMalloc(sizeof(struct msg_struct *));
	msg->msg = pvPortMalloc(sizeof(char *));

	int random_len = 0;

	xLastWakeTime = xTaskGetTickCount();

	/* Define a task that performs an action every random milisecons starting (at least 20)*/
	const TickType_t xPeriod = pdMS_TO_TICKS((rand() % (450 - 150) + 150));
	/*Esta tarea dura entre 20 ms y 150*/
	// rand()%(nMax-nMin) + nMin;
	for (;;) {
		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time. */
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		random_len = (rand() + xTaskGetTickCount()) % (MAX_STRING_LEN - 3);

		/* Send to the queue - causing the queue receive task to flash its LED.*/
		msg->msg = vRandomString(random_len);
		msg->len_str = random_len+3;

		/*Envio string, esperando hasta 5[ms]  si la cola esta llena
		 *
		 * xQueueSend(), xQueueSendToFront() and xQueueSendToBack() will
		 * return immediately if xTicksToWait is zero and the queue is already full.
		 *
		 * */
		if (xQueueSend( xPointerQueue, &msg, pdMS_TO_TICKS(10) ) != pdTRUE) {
			printf("\nCould not post data [PERIODIC - TASK]\n");
		}
		vPortFree(msg->msg);
	}

}

/**
 * Tarea de recepcion. Se fija en la cola si hay algo y lo recibe. Compara campos de la estructura, y lo
 * reenvia por puerto serie.
 *
 *
 * @param void*  pvParameters
 */

static void vReceiverTask(void *pvParameters) {

	struct msg_struct * pcReceivedValue;


	for (;;) {
		/* Wait until something arrives in the queue - this task will block
		 indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		 FreeRTOSConfig.h. */

		/* Receive the address of a buffer. */
		if (xQueueReceive(xPointerQueue, /* The handle of the queue. */
		&pcReceivedValue, /* Store the buffer’s address in pcReceivedString. */
		portNEW_DELAY) == pdTRUE) {

			if (pcReceivedValue->msg == NULL) {
				/*Vino valor de temperatura*/
				printf("\nTemperature meassure: %d C\n", pcReceivedValue->num);
				/*Free a estructura de recepecion*/
				vPortFree(pcReceivedValue);


			} else {

				/*Vino valor de temperatura*/
				/* Print out the name of this task. */
				vPrintString("Variable generated string:");
				vPrintString(pcReceivedValue->msg);
				vPrintString("\n");

				/*Free a estructura*/
				vPortFree(pcReceivedValue);

			}

			/* toggle LED 22 */
			vToggleLED();

			/*Free a estructura*/

		} else {
			/* Returned if data cannot be read from the queue because the queue is
			 * already empty.
			 *
			 * If a block time was specified (xTicksToWait was not zero) then the
			 * calling task will have been placed into the Blocked state to wait for
			 * another task or interrupt to send data to the queue, but the block time
			 * expired before this happened.
			 *
			 */
			printf("\nQueue empty. No data to read\n\n");
		}

	}

}

/*
 * Funcion para generar string de longitud variable. No solo varia la longitud de
 * la cadena, sino que la posicion dentro del arreglo fijo, i.e, si genero una cadena
 * de largo 5, no implica que sean los primeros 5 caracteres siempre.
 *
 * @param: len_str longitud de la cadena devuelta
 * @return: char*	cadena generada
 *
 * */

char * vRandomString(int len_string) {
	static char* leters = "a0b1c2d3e4f5g6h7i8j9k0l1m2n3o4p5q6r7s8t9u0v1w2x3y4z5";

	/* +3 para el agregado de \r\n */
	char* var_string = pvPortMalloc(len_string += 3);

	for (int i = 0; i < len_string - 3; i++) {
		var_string[i] = leters[rand() % MAX_STRING_LEN]; //elige elemento con indice de 0 a 50
	}
	var_string[len_string - 3] = '\r';
	var_string[len_string - 2] = '\n';
	var_string[len_string - 1] = '\0';

	return var_string;
}



/**
 * toggle LED 22
 */
static void vToggleLED(void) {
	unsigned long ulLEDState;

	/* Obtain the current P0 state. */
	ulLEDState = LPC_GPIO0->FIOPIN;

	/* Turn the LED off if it was on, and on if it was off. */
	LPC_GPIO0->FIOCLR = ulLEDState & (1 << mainLED_BIT);
	LPC_GPIO0->FIOSET = ((~ulLEDState) & (1 << mainLED_BIT));
}


void vConfigureTimerForRunTimeStats(void) {
	const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00,
			TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	 collecting run time statistical information - basically the percentage
	 of CPU time that each task is utilising.  It is called automatically when
	 the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	 to 1). */

	/* Power up and feed the timer. */
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3 << 2))) | (0x01 << 2);

	/* Reset Timer 0 */
	LPC_TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	 but not too fast so as to overflow all the time. */
	LPC_TIM0->PR = ( configCPU_CLOCK_HZ / 10000UL) - 1UL;

	/* Start the counter. */
	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}

/*
 * Necessary functions for FreeRTOS
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
	/* This function will get called if a task overflows its stack. */
	(void) pxTask;
	(void) pcTaskName;
	for (;;)
		;
}


int consoleprint(char *cpstring)
{
	 int slen, res;
	 // Calculate length of string
	 slen = strlen (cpstring);
	 // Call function to carry out semihosting "SYS_WRITE" operation
	 res = LIBSTUB_SYS_WRITE (0, cpstring,slen);	// Note that '0' represents stdout
	 return res;
}


void vPrintString( const char *pcString )
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		sprintf( cBuffer, "%s", pcString );
		consoleprint( cBuffer );
	}
	xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vPrintStringAndNumber( const char *pcString, unsigned long ulValue )
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		sprintf( cBuffer, "%s %lu\n", pcString, ulValue );
		consoleprint( cBuffer );
	}
	xTaskResumeAll();
}


