/*
 * Task.c
 *
 *  Created on: Oct 5, 2024
 *      Author: Zyad Montaser
 */
#include <Task.h>

/*********************************************************************************************
                                        *Tasks Defintions*
*********************************************************************************************/
typedef struct {
    uint8 CurrentTemperature;
    Heater_States HeaterState;
    Heating_Levels HeatingLevel;
} SystemStatus;

SystemStatus gSystemStatus;

void GPIOPortF_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    static Button_Press_Times Heating_Level_Button=NO_PRESS;/*counter to count the button presses*/

    if(isDebounce == FALSE )
    {
        /*  Clearing the interrupt flag for PF4 == > SW1    */
        SET_BIT(GPIO_PORTF_ICR_REG , 4);

        if( xSemaphoreTakeFromISR(UART_MUTEX,&xHigherPriorityTaskWoken) == pdPASS )
        {
            UART0_SendString("\nGPIOPortF_Handler is started\r\n");
        }
        xSemaphoreGiveFromISR(UART_MUTEX,&xHigherPriorityTaskWoken);

        Heating_Level_Button++;
        if(Heating_Level_Button==FIFTH_PRESS)
        {
            Heating_Level_Button=FIRST_PRESS;
        }
        xQueueSendFromISR(ButtonPressedTimes_Queue, &Heating_Level_Button, &xHigherPriorityTaskWoken);

        xSemaphoreGiveFromISR(GetDesiredTemperatureTask_SEMAPHORE, &xHigherPriorityTaskWoken);

        /* Clear Trigger flag for PF0 (Interrupt Flag) */
        GPIO_PORTF_ICR_REG   |= (1<<0);

        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

void GetDesiredTemperatureTask(void *pvParameters)
{
    Heating_Levels Heating_Level;
    Button_Press_Times Heating_Level_Button;

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("GetDesiredTemperature task is started\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if(xSemaphoreTake(GetDesiredTemperatureTask_SEMAPHORE,portMAX_DELAY)==pdPASS)
        {
            //UART0_SendString("ButtonSemaphore is taked sucessfully \r\n");

            xQueueReceive(ButtonPressedTimes_Queue, &Heating_Level_Button, portMAX_DELAY);

//            if(xQueueReceive(ButtonPressedTimes_Queue, &Heating_Level_Button, portMAX_DELAY)==pdPASS)
//            {
//                UART0_SendString("ButtonPressedTimes is received sucessfully \r\n");
//            }
//            else
//            {
//                UART0_SendString("ButtonPressedTimes didn't get received \r\n");
//            }

            switch(Heating_Level_Button)
            {
            case FIRST_PRESS:
                Heating_Level=LOW_HEATING;
                break;
            case SECOND_PRESS:
                Heating_Level=MEDIUM_HEATING;
                break;
            case THIRD_PRESS:
                Heating_Level=HIGH_HEATING;
                break;
            case FOURTH_PRESS:
                Heating_Level=NO_HEATING;
                break;
            }
            if(xQueueSend(HeatingLevel_Queue, &Heating_Level, portMAX_DELAY)==pdTRUE)
            {
                gSystemStatus.HeatingLevel=Heating_Level;
                //UART0_SendString("HeatingLevel sent sucessfully \r\n");
            }
//            else
//            {
//                UART0_SendString("HeatingLevel didn't get sent \r\n");
//            }
            xSemaphoreGive(GetCurrentTemperatureTask_SEMAPHORE);
        }
    }
}

void GetCurrentTemperatureTask(void *pvParameters)
{
    uint32 ADC_value ;
    uint8 CurrentTemperature;

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("GetCurrentTemperature task is started\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if(xSemaphoreTake(GetCurrentTemperatureTask_SEMAPHORE,portMAX_DELAY)==pdPASS)
        {
            ADC_value=ADC0_Read_AIN1_PE2();

            // Debug ADC value
//            if (xSemaphoreTake(UART_MUTEX, portMAX_DELAY) == pdPASS)
//            {
//                UART0_SendString("ADC Value: ");
//                UART0_SendInteger(ADC_value);
//                UART0_SendString("\r\n");
//            }
//            xSemaphoreGive(UART_MUTEX);

            /*based on the given specification 45°C-->3.3V-->4095
             *                                  T ---------->Digital so T=(Digital*45)/4095=Digital/91  */
            CurrentTemperature=(uint8)((ADC_value)/91);

            // Debug current temperature
//            if (xSemaphoreTake(UART_MUTEX, portMAX_DELAY) == pdPASS) {
//                UART0_SendString("Current Temperature: ");
//                UART0_SendInteger(CurrentTemperature);
//                UART0_SendString("\r\n");
//            }
//            xSemaphoreGive(UART_MUTEX);

            if( CurrentTemperature <5 || CurrentTemperature>40 )
            {
                xSemaphoreGive(FailureHandlingTask_SEMAPHORE);
            }
            else
            {
                if(xQueueSend(CurrentTemperature_Queue,&CurrentTemperature,portMAX_DELAY)==pdTRUE)
                {
                    gSystemStatus.CurrentTemperature=CurrentTemperature;
                    //UART0_SendString("CurrentTemperature is sent sucessfully \r\n");
                }
//                else
//                {
//                    UART0_SendString("failed to send the CurrentTemperature \r\n");
//                }
            }
            xSemaphoreGive(HeaterControlTask_SEMAPHORE);
        }
    }
}
void FailureHandlingTask(void *pvParameters)
{

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("FailureHandling task is started\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if(xSemaphoreTake(FailureHandlingTask_SEMAPHORE,portMAX_DELAY)== pdPASS)
        {
            UART0_SendString("The temperature is out of range \r\n");
            GPIO_BlueLedOff();
            GPIO_GreenLedOff();
            GPIO_RedLedOn();
        }
//        else
//        {
//            UART0_SendString("SensorErrorSemaphore fail to be taken\r\n");
//        }
    }
}
void HeaterControlTask(void *pvParameters)
{
    uint8 Recieved_Heating_Level;
    Heater_States Heater_State=HEATER_OFF;
    uint8 Recieved_CurrentTemperature;
    uint8 difference;

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("HeaterControl task is started\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if(xSemaphoreTake(HeaterControlTask_SEMAPHORE,portMAX_DELAY)== pdPASS)
        {
            xQueueReceive(CurrentTemperature_Queue, &Recieved_CurrentTemperature, portMAX_DELAY);

//            if(xQueueReceive(CurrentTemperature_Queue, &Recieved_CurrentTemperature, portMAX_DELAY)== pdPASS)
//            {
//                if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
//                {
//                    UART0_SendString("CurrentTemperature before subtraction = ");
//                    UART0_SendInteger(Recieved_CurrentTemperature);
//                    UART0_SendString("\r\n");
//                }
//                xSemaphoreGive(UART_MUTEX);
//            }
//            else
//            {
//                UART0_SendString("CurrentTemperature didn't get received \r\n");
//            }

            xQueueReceive(HeatingLevel_Queue, &Recieved_Heating_Level, portMAX_DELAY);
//            if(xQueueReceive(HeatingLevel_Queue, &Recieved_Heating_Level, portMAX_DELAY)== pdPASS)
//            {
//                if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
//                {
//                    UART0_SendString("HeatingLevel before subtraction = ");
//                    UART0_SendInteger(Recieved_Heating_Level);
//                    UART0_SendString("\r\n");
//                }
//                xSemaphoreGive(UART_MUTEX);
//            }
//            else
//            {
//                UART0_SendString("HeatingLevel didn't get received \r\n");
//            }

            difference=Recieved_Heating_Level - Recieved_CurrentTemperature;

//            if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
//            {
//                UART0_SendString("Differnce after subtraction= ");
//                UART0_SendInteger(difference);
//                UART0_SendString("\r\n");
//            }
//            xSemaphoreGive(UART_MUTEX);

            if(Recieved_CurrentTemperature >= Recieved_Heating_Level)
            {
                Heater_State=HEATER_OFF;
                GPIO_BlueLedOff();
                GPIO_GreenLedOn();
                GPIO_RedLedOn();
            }

            else if( difference>10)
            {
                Heater_State=HEATER_HIGH_INTENSITY;
                GPIO_BlueLedOn();
                GPIO_GreenLedOn();
                GPIO_RedLedOff();
            }
            else if( difference>5 && difference<=10)
            {
                Heater_State=HEATER_MEDIUM_INTENSITY;
                GPIO_BlueLedOn();
                GPIO_GreenLedOff();
                GPIO_RedLedOff();
            }
            else if( difference>=2 && difference<=5)
            {
                Heater_State=HEATER_LOW_INTENSITY;
                GPIO_GreenLedOn();
                GPIO_RedLedOff();
                GPIO_BlueLedOff();
            }
            else
            {
                Heater_State=HEATER_OFF;
                GPIO_RedLedOff();
                GPIO_BlueLedOff();
                GPIO_GreenLedOff();
            }

            gSystemStatus.HeaterState=Heater_State;/*Send the heater state to the struct to be received by DisplayScreenTask*/

            xSemaphoreGive(DisplayScreenTask_SEMAPHORE);
        }
    }
}

void DisplayScreenTask(void *pvParameters)
{
    uint8 CurrentTemperature;
    Heater_States Heater_State;
    Heating_Levels Heating_Level;

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("DisplayScreen task is started\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if(xSemaphoreTake(DisplayScreenTask_SEMAPHORE,portMAX_DELAY)== pdPASS)
        {
            CurrentTemperature=gSystemStatus.CurrentTemperature;
            Heater_State=gSystemStatus.HeaterState;
            Heating_Level=gSystemStatus.HeatingLevel;

            UART0_SendString("****************************************************\r\n");
            UART0_SendString("Current Temperature: ");
            UART0_SendInteger(CurrentTemperature);
            UART0_SendString("\r\n");

            switch(Heater_State)
            {
            case HEATER_OFF:
                UART0_SendString("Heater State: off");
                UART0_SendString("\r\n");
                break;

            case HEATER_LOW_INTENSITY:
                UART0_SendString("Heater State: Low intensity");
                UART0_SendString("\r\n");
                break;

            case HEATER_MEDIUM_INTENSITY:
                UART0_SendString("Heater State: Medium intensity");
                UART0_SendString("\r\n");
                break;

            case HEATER_HIGH_INTENSITY:
                UART0_SendString("Heater State: High intensity");
                UART0_SendString("\r\n");
                break;
            }

            switch(Heating_Level)
            {
            case NO_HEATING:
                UART0_SendString("Required Heating Level: NO HEATING");
                UART0_SendString("\r\n");
                break;
            case LOW_HEATING:
                UART0_SendString("Required Heating Level: LOW HEATING");
                UART0_SendString("\r\n");
                break;
            case MEDIUM_HEATING:
                UART0_SendString("Required Heating Level: MEDIUM HEATING");
                UART0_SendString("\r\n");
                break;
            case HIGH_HEATING:
                UART0_SendString("Required Heating Level: HIGH HEATING");
                UART0_SendString("\r\n");
                break;
            }
            xSemaphoreGive(RunTimeMeasurementsTask_SEMAPHORE);
        }
    }
}

void RunTimeMeasurementsTask(void *pvParameters)
{

    if( xSemaphoreTake(UART_MUTEX,portMAX_DELAY) == pdPASS )
    {
        UART0_SendString("RunTimeMeasurements task is started\r\n");
        UART0_SendString("****************************************************\r\n");
    }
    xSemaphoreGive(UART_MUTEX);

    while(1)
    {
        if( xSemaphoreTake(RunTimeMeasurementsTask_SEMAPHORE,portMAX_DELAY) == pdPASS )
        {
            uint8 ucCounter, ucCPU_Load;
            uint32 ullTotalTasksTime = 0;
            for(ucCounter = 1; ucCounter < 7; ucCounter++)
            {
                switch(ucCounter)
                {
                case GetDesiredTemperatureTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("\n**************************************Run Time Measurements*************************************\r\n");
                    UART0_SendString("GetDesiredTemperatureTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                case HeaterControlTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("HeaterControlTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                case GetCurrentTemperatureTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("GetCurrentTemperatureTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                case DisplayScreenTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("DisplayScreenTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                case FailureHandlingTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("FailureHandlingTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                case RunTimeMeasurementsTask_TAG:
                    taskENTER_CRITICAL();
                    UART0_SendString("RunTimeMeasurementsTask Execution time = ");
                    UART0_SendInteger(ullTaskExecutiontime[ucCounter]);
                    UART0_SendString("ms \r\n");
                    taskEXIT_CRITICAL();
                    break;
                }
                ullTotalTasksTime += ullTasksTotalTime[ucCounter];
            }
            ucCPU_Load = (ullTotalTasksTime * 100) /  GPTM_WTimer0Read();

            taskENTER_CRITICAL();
            UART0_SendString("\nCPU Load is ");
            UART0_SendInteger(ucCPU_Load);
            UART0_SendString("% \r\n");
            taskEXIT_CRITICAL();
        }
    }
}


/*  Timer callback to reset Debounce flag    */
void DebounceTimerCallback(TimerHandle_t xTimer)
{

    /* Reset the Debouncing flag, allowing new interrupts to be processed   */
    isDebounce = FALSE ;

}

