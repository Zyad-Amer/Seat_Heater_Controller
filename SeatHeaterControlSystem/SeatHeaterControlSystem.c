
#include <Task.h>

/* The HW setup function */
static void prvSetupHardware( void );

int main(void)
{
    /* Setup the hardware for use with the Tiva C board. */
    prvSetupHardware();


    /*******Creating Software Timers*******/
    /*  Create a software timer for Debouncing (50ms delay) */
    xDebounceTimerHandle = xTimerCreate("Debounce Timer",
                                           pdMS_TO_TICKS(DEBOUNCE_DELAY_MS),
                                           pdFALSE,
                                           (void *)0,
                                           DebounceTimerCallback);

    UART0_SendString("*******************************************************\r\n");
    UART0_SendString("                                By Zyad Montaser\r\n");
    UART0_SendString("*******************************************************\r\n\n");
    if( xDebounceTimerHandle != NULL )
    {
        UART0_SendString("Debounce Timer Created\r\n");
    }
    else
    {
        UART0_SendString("Debounce Timer Failed\r\n");
    }


    /* Create Tasks here */
    xTaskCreate(GetDesiredTemperatureTask, "GetDesiredTemperatureTask", 256, NULL, 4, &GetDesiredTemperatureTask_Handle);

    xTaskCreate(HeaterControlTask, "HeaterControlTask", 256, NULL, 2, &HeaterControlTask_Handle);

    xTaskCreate(GetCurrentTemperatureTask, "GetCurrentTemperatureTask", 256, NULL, 3, &GetCurrentTemperatureTask_Handle);

    xTaskCreate(DisplayScreenTask, "DisplayScreenTask", 256, NULL, 1, &DisplayScreenTask_Handle);

    xTaskCreate(FailureHandlingTask, "FailureHandlingTask", 256, NULL, 4, &FailureHandlingTask_Handle);

    xTaskCreate(RunTimeMeasurementsTask, "RunTimeMeasurementsTask", 256, NULL, 0, &RunTimeMeasurementsTask_Handle);

    vTaskSetApplicationTaskTag( GetDesiredTemperatureTask_Handle ,( TaskHookFunction_t ) 1 );
    vTaskSetApplicationTaskTag( HeaterControlTask_Handle         ,( TaskHookFunction_t ) 2 );
    vTaskSetApplicationTaskTag( GetCurrentTemperatureTask_Handle ,( TaskHookFunction_t ) 3 );
    vTaskSetApplicationTaskTag( DisplayScreenTask_Handle         ,( TaskHookFunction_t ) 4 );
    vTaskSetApplicationTaskTag( FailureHandlingTask_Handle       ,( TaskHookFunction_t ) 5 );
    vTaskSetApplicationTaskTag( RunTimeMeasurementsTask_Handle   ,( TaskHookFunction_t ) 6 );

    GetDesiredTemperatureTask_SEMAPHORE=xSemaphoreCreateBinary();
    FailureHandlingTask_SEMAPHORE=xSemaphoreCreateBinary();
    GetCurrentTemperatureTask_SEMAPHORE=xSemaphoreCreateBinary();
    HeaterControlTask_SEMAPHORE=xSemaphoreCreateBinary();
    DisplayScreenTask_SEMAPHORE=xSemaphoreCreateBinary();
    RunTimeMeasurementsTask_SEMAPHORE=xSemaphoreCreateBinary();

    UART_MUTEX = xSemaphoreCreateMutex();

    HeatingLevel_Queue = xQueueCreate(4, sizeof(uint8));
    CurrentTemperature_Queue = xQueueCreate(4,sizeof(uint8));
    HeaterState_Queue=xQueueCreate(4,sizeof(uint8));
    ButtonPressedTimes_Queue=xQueueCreate(4,sizeof(uint8));


    /* Now all the tasks have been created - start the scheduler*/
    vTaskStartScheduler();

    /* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
    for (;;);

}

static void prvSetupHardware( void )
{
    /* Place here any needed HW initialization such as GPIO, UART, etc.  */
    GPIO_BuiltinButtonsLedsInit();
    UART0_Init();
    ADC0_Init();
    GPTM_WTimer0Init();
    GPIO_SW1EdgeTriggeredInterruptInit();
}
