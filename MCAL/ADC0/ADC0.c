/**********************************************************************************************
 *
 * Module: ADC0
 *
 * File Name: ADC0.c
 *
 * Description: Source file for the TM4C123GH6PM ADC0 driver for TivaC
 *
 * Author: Zyad Montaser
 *
 ***********************************************************************************************/
#include <ADC0/ADC0.h>
#include "common_macros.h"
#include "tm4c123gh6pm_registers.h"

void ADC0_Init()
{


    /*  1   */
    /*  Enable the ADC0 clock using the RCGCADC register and waiting  */

    SET_BIT(ADC0_RCGCADC_REG , 0);
    while (!GET_BIT(SYSCTL_PRADC_REG, 0)); // Wait for ADC0 to be ready


    /*  2   */
    /*  Enabling the Clock For PortE    */

    SET_BIT(SYSCTL_RCGCGPIO_REG , 4);
    while(!GET_BIT(SYSCTL_PRGPIO_REG, 4));

    /*  3   */
    /*  Choosing Analog Mode For Pins PE3 & PE2   */

    SET_BIT(GPIO_PORTE_AFSEL_REG , 2); // PE2 == > AIN1
    SET_BIT(GPIO_PORTE_AFSEL_REG , 3); // PE3 == > AIN0

    /*  4   */
    /*  Configure the AINx signals to be Analog Inputs  */

    CLEAR_BIT(GPIO_PORTE_DEN_REG , 2); // PE2 == > AIN1
    CLEAR_BIT(GPIO_PORTE_DEN_REG , 3); // PE3 == > AIN0

    /*  5   */
    /*   Disable the analog isolation circuit for all ADC input pins    */

    SET_BIT(GPIO_PORTE_AMSEL_REG , 2); // PE2 == > AIN1
    SET_BIT(GPIO_PORTE_AMSEL_REG , 3); // PE3 == > AIN0

    /*  For Module Clocking , Adjusting ADC Clock on PIOSC providing a 16-MHz clock source  */

        ADC0_ADCCC_REG = ( ( ADC0_ADCCC_REG & 0x00000000 ) | 0x1 ) ;


    /*  Programming Sample Sequencer  SS2  */

    /*  1- Disabling Sample Sequencer To Program It */

        CLEAR_BIT(ADC0_ADCACTSS_REG , 2);


    /*  2- Configure the trigger event for the sample sequencer in the ADCEMUX register.    */

    ADC0_ADCEMUX_REG = ( (ADC0_ADCEMUX_REG & 0xFFFFFFF0) | 0xF ) ;

    /*  3- Configure the corresponding input source in the ADCSSMUXn register   */

    /*    Clearing it     */
    /*  Choosing MUX0 (PE3) == > AIN0 and MUX1 (PE2) == > AIN1    */

    ADC0_ADCSSMUX2_REG = ( ( ADC0_ADCSSMUX2_REG & 0x00000000 ) | 0x0010 ) ;


    /*  4- configure the sample control bits in the corresponding nibble in the ADCSSCTLn register  */

    ADC0_ADCSSCTL2_REG = 0x006 ;

    /*  5- Enabling Sample Sequencer SS2   */

    SET_BIT(ADC0_ADCACTSS_REG , 2);


}


//void ADC0_init()
//{
//    SYSCTL_RCGCADC_REG|= 0x01;           /* Enable clock for ADC0 */
//    while(!(SYSCTL_RCGCADC_REG & 0x01)); /* Wait until ADC0 clock is activated and it is ready for access*/
//
//    SYSCTL_RCGCGPIO_REG  |= 0x10;        /* Enable clock for GPIO PORTE */
//    while(!(SYSCTL_PRGPIO_REG & 0x10));  /* Wait until GPIO PORTE clock is activated and it is ready for access*/
//
//    GPIO_PORTE_AFSEL_REG |= (1<<3);      /* Enable alternative function on PE3 */
//    GPIO_PORTE_DEN_REG   &= ~(1<<3);      /* Disable Digital on PE3 */
//    GPIO_PORTE_AMSEL_REG |= (1<<3);      /* Enable Analog on PE3 */
//
//    /*  For Module Clocking , Adjusting ADC Clock on PIOSC providing a 16-MHz clock source  */
//    ADC0_ADCCC_REG = ( ( ADC0_ADCCC_REG & 0x00000000 ) | 0x1 ) ;
//
//    CLEAR_BIT(ADC0_ADCACTSS_REG , 2);/* Disable Sample sequencer 2 during configuration */
//    ADC0_ADCEMUX_REG = ( (ADC0_ADCEMUX_REG & 0xFFFFFFF0) | 0xF ) ;/*Configure the trigger event for SS2*/
//    ADC0_ADCSSMUX2_REG = ( ( ADC0_ADCSSMUX2_REG & 0x00000000 ) | 0x0010 ) ;/* Get the input from channel 0 (PE3)*/
//    ADC0_ADCSSCTL2_REG = 0x006 ;/*configure the sample control bits in the corresponding nibble*/
//    SET_BIT(ADC0_ADCACTSS_REG , 2);/*Enabling Sample Sequencer SS2*/
//
//}

//uint16 ADC0_ReadChannel()
//{
//    ADC0_ADCSSMUX2_REG = 0x0; // MUX0 selects AIN0 (PE3)
//    ADC0_ADCSSCTL2_REG = 0x6; // Set SS2 for single-ended input and EOS for AIN0
//
//    SET_BIT(ADC0_ADCPSSI_REG, 2); // Start conversion for SS2
//
//    while (!GET_BIT(ADC0_ADCRIS_REG, 2)); // Wait until conversion is complete
//
//    uint16 result = ADC0_ADCSSFIFO2_REG; // Read AIN0 result (PE3)
//
//    SET_BIT(ADC0_ADCISC_REG, 2); // Clear the completion flag for SS2
//
//    return result; // Return the AIN0 result
//}
uint16 ADC0_Read_AIN1_PE2()
{
    /*  1. Configure SS2 for AIN1 (PE2) */
    ADC0_ADCSSMUX2_REG = 0x1; // MUX0 selects AIN1 (PE2)
    ADC0_ADCSSCTL2_REG = 0x6; // Set SS2 for single-ended input and EOS for AIN1

    /*  2. Trigger the conversion on Sample Sequencer 2 (SS2) */
    SET_BIT(ADC0_ADCPSSI_REG, 2); // Start conversion for SS2

    /*  3. Wait for the conversion to complete */
    while (!GET_BIT(ADC0_ADCRIS_REG, 2)); // Wait until conversion is complete

    /*  4. Read the conversion result from the FIFO for SS2 */
    uint16 result = ADC0_ADCSSFIFO2_REG; // Read AIN1 result (PE2)

    /*  5. Clear the interrupt flag for SS2 */
    SET_BIT(ADC0_ADCISC_REG, 2); // Clear the completion flag for SS2

    return result; // Return the AIN1 result
}
