/**********************************************************************************************
 *
 * Module: ADC0
 *
 * File Name: ADC0.h
 *
 * Description: Header file for the TM4C123GH6PM ADC driver for TivaC
 *
 * Author: Zyad Montaser
 *
 ***********************************************************************************************/

#ifndef ADC0_H_
#define ADC0_H_

#include "std_types.h"

void ADC0_init();
uint16 ADC0_ReadChannel();

void ADC0_Init();
uint16 ADC0_Read_AIN1_PE2();

#endif /* ADC0_H_ */
