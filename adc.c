#include "p33FJ16GS502.h"
#include <math.h>

#include "adc.h"




WORD adcCntrVolt = 0;
WORD adcCntrCurr = 0;
BYTE bBufferFull = 0;
WORD inSamplesCurr[SMP_BUFFER];
WORD inSamplesVolt[SMP_BUFFER];
WORD inSamplesPow [SMP_BUFFER];

UINT32 voltageAcc = 0;
UINT32 currentAcc = 0;
double powerAcc = 0;
double vinst, iinst;
int numSamples = 0;
UINT16 temp;

BOOL bInt1 = FALSE;

void initAD(void) {

    _LATA0 = 0;
    _TRISA0 = 1;
    _LATA1 = 0;
    _TRISA1 = 1;

    ADCON = 0x0000; // Turn off the A/D converter
    ADCONbits.FORM = 0; //INTEGER)
    ADCONbits.EIE = 0; //early interrupt
    ADCONbits.ORDER = 0; // Normal Order of Conversion
    ADCONbits.SEQSAMP = 0; // Simultaneous Sampling
    ADCONbits.ASYNCSAMP = 0; // Asynchronous Sampling
    ADCONbits.SLOWCLK = 0; // High Frequency Clock Input
    ADCONbits.ADCS = 3; // Clock Divider Selection
    ADSTAT = 0;
    ADCPC0bits.TRGSRC0 = 0b11111; //timer 2 sampling
    ADCPC0bits.IRQEN0 = 1;
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG0 = 0; // AN0 is configured as analog input
    ADPCFGbits.PCFG1 = 0; // AN1 is configured as analog input


    IPC27bits.ADCP0IP = 0x02; // Set ADC Pair 0 Interrupt Priority (Level 1)
    IFS6bits.ADCP0IF = 0; // Clear ADC Pair 0 Interrupt Flag
    IEC6bits.ADCP0IE = 1; // disable ADC Pair 0 Interrupt



    //Configura Timer 2
    //***   Timer2  50us
    T2CONbits.TON = 0;
    TMR2 = 0x0000;
    PR2 = SMPERIODO; //625us / 16 =   39

    //Start Timer 2
    T2CONbits.TCKPS = 0b11; //   cada 62.5 ns * 256 = 16 us

    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1; //T3 ON


    ADCONbits.ADON = 1; // Turn off the A/D converter


}
//***********************************************************************

BOOL getBufferFull() {
    return bBufferFull;

}
//**********************************************************************

void setBufferFull(BOOL bBuff) {
    bBufferFull = bBuff;

}
//*********************************************************************+**

WORD maxValor(WORD arrayAD[], BYTE *counter) {
    WORD max;
    int i;

    BYTE index = 0;

    max = 0;
    for (i = 0; i < SMP_BUFFER - 1; i++) {

        if (arrayAD[i] > max) {
            max = arrayAD[i];
            index++;
        }
    }
    *counter = index;

    return max;


}
//************************************************************************

void startMedidas() {
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1; //T2 ON
    //IFS6bits.ADCP0IF = 0;
    IEC6bits.ADCP0IE = 1; // Enable ADC Pair 0 Interrupt
    ADCONbits.ADON = 1;
    bBufferFull = FALSE;

}
//************************************************************************

void stopMedidas() {
    IEC0bits.T2IE = 0;
    T2CONbits.TON = 0; //T2 OFF
    IEC6bits.ADCP0IE = 0; // disable ADC Pair 0 Interrupt
    ADCONbits.ADON = 0;
    adcCntrVolt = 0;
    adcCntrCurr = 0;
    numSamples = 0;
    bBufferFull = TRUE;

}
//************************************************************************

// Interrupt Service Routine code goes here

void __attribute__((interrupt, no_auto_psv)) _ADCP0Interrupt(void) {


    inSamplesVolt[adcCntrVolt++] = ADCBUF0; //ReadADC10( 1 );

    inSamplesCurr[adcCntrCurr++] = ADCBUF1;

    temp = ADCBUF0;
    if (temp > 512) temp -= 512;
    else if (temp < 512) temp = 512 - temp;


    vinst = (temp * K_2R2 * K_AD) * K_TRAFO * K_DIV;
    voltageAcc += vinst * vinst; //pow(vinst, 2);

    temp = ADCBUF1;
    if (temp > 512) temp -= 512;
    else if (temp < 512) temp = 512 - temp;

    iinst = (temp * K_2R2 * K_AD) * K_AMP;
    currentAcc += iinst * iinst;

    powerAcc += iinst * vinst;

    if (++numSamples >= SMP_BUFFER) {

        ADCONbits.ADON = 0;
        IEC6bits.ADCP0IE = 0; // disable ADC Pair 0 Interrupt

        adcCntrVolt = 0;
        adcCntrCurr = 0;
        numSamples = 0;
        bBufferFull = TRUE;

    }
    IFS6bits.ADCP0IF = 0; // Clear ADC Pair 0 Interrupt Flag
}

void _ISR __attribute__((__no_auto_psv__)) _T2Interrupt(void) {


    static int i = 0;


    i++;


    IFS0bits.T2IF = 0;


}



