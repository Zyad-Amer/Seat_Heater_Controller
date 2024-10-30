/*
 * Task.h
 *
 *  Created on: Oct 5, 2024
 *      Author: Zyad Montaser
 */

#ifndef TASK_H_
#define TASK_H_

/***************************************** Kernel includes *********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/***************************************** MCAL includes *********************************/
#include "gpio.h"
#include "uart0.h"
#include "GPTM.h"
#include <ADC0/ADC0.h>
#include "tm4c123gh6pm_registers.h"
#include "common_macros.h"

/*   Adjust this value based on the bouncing time     */
#define DEBOUNCE_DELAY_MS           (100)

#define GetDesiredTemperatureTask_TAG       1
#define HeaterControlTask_TAG               2
#define GetCurrentTemperatureTask_TAG       3
#define DisplayScreenTask_TAG               4
#define FailureHandlingTask_TAG             5
#define RunTimeMeasurementsTask_TAG         6

/***************************************** Tasks prototypes *********************************/
void GetDesiredTemperatureTask(void *pvParameters);
void HeaterControlTask(void *pvParameters);
void GetCurrentTemperatureTask(void *pvParameters);
void DisplayScreenTask(void *pvParameters);
void FailureHandlingTask(void *pvParameters);
void RunTimeMeasurementsTask(void *pvParameters);

void DebounceTimerCallback(TimerHandle_t xTimer);

/***************************************** Handlers prototypes *********************************/
void GPIOPortF_Handler(void);


/****************************************** Definitions *********************************/
uint32 ullTasksOutTime[7];
uint32 ullTasksInTime[7];
uint32 ullTasksTotalTime[7];
uint32 ullTaskExecutiontime[7];

/*  Debouncing Flag     */
static volatile uint8 isDebounce = FALSE ;

/* Used to hold the handle of tasks */
TaskHandle_t GetDesiredTemperatureTask_Handle;
TaskHandle_t HeaterControlTask_Handle;
TaskHandle_t GetCurrentTemperatureTask_Handle;
TaskHandle_t DisplayScreenTask_Handle;
TaskHandle_t FailureHandlingTask_Handle;
TaskHandle_t RunTimeMeasurementsTask_Handle;

/*  Software Timer handle that is used for debouncing the interrupt */
TimerHandle_t xDebounceTimerHandle ;

xSemaphoreHandle GetDesiredTemperatureTask_SEMAPHORE;
xSemaphoreHandle FailureHandlingTask_SEMAPHORE;
xSemaphoreHandle GetCurrentTemperatureTask_SEMAPHORE;
xSemaphoreHandle HeaterControlTask_SEMAPHORE;
xSemaphoreHandle DisplayScreenTask_SEMAPHORE;
xSemaphoreHandle RunTimeMeasurementsTask_SEMAPHORE;

xSemaphoreHandle UART_MUTEX ;

QueueHandle_t ButtonPressedTimes_Queue;
QueueHandle_t HeatingLevel_Queue;
QueueHandle_t CurrentTemperature_Queue;
QueueHandle_t HeaterState_Queue;

/* The heating levels of the user button*/
typedef enum
{
    NO_HEATING,
    LOW_HEATING=25,
    MEDIUM_HEATING=30,
    HIGH_HEATING=35,
}Heating_Levels;

typedef enum
{
    HEATER_OFF,
    HEATER_LOW_INTENSITY,
    HEATER_MEDIUM_INTENSITY,
    HEATER_HIGH_INTENSITY,
}Heater_States;

typedef enum
{
    NO_PRESS,
    FIRST_PRESS,
    SECOND_PRESS,
    THIRD_PRESS,
    FOURTH_PRESS,
    FIFTH_PRESS
}Button_Press_Times;

#endif /* TASK_H_ */
