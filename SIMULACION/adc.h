/* 
 * File:   adc.h
 * Author: t133643
 *
 * Created on 4 de septiembre de 2012, 12:16
 */

#ifndef ADC_H
#define	ADC_H

#include "GenericTypeDefs.h"


#define SMP_BUFFER   128     //32 muestras
#define SMPERIODO    20 //39     //100 Hz                    // (FCY/FSAMPLE) - 1 ????
#define VDD          3.3
#define K_TRAFO      17
#define K_DIV        6.66
#define K_AD         VDD/1024
#define K_2R2        2.81         //2 x 1.414
#define K_AMP        6.8
#define K_CS60       1.61

void initAD(void);
BOOL getBufferFull();
void setBufferFull(BOOL estado);
void startMedidas();
void stopMedidas();



#endif	/* ADC_H */

