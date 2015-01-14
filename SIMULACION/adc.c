#include <p33FJ32GP202.h>
#include "GenericTypeDefs.h"
#include "adc.h"





WORD inSamplesCurr[SMP_BUFFER];
WORD inSamplesVolt[SMP_BUFFER];
UINT32 voltageAcc = 0;
UINT32 currentAcc = 0;
double powerAcc = 0;
UINT16 temp;
WORD maxVolt;

double vinst, iinst;
WORD adcCntr = 0;
BYTE bBufferFull = 0;

void initAD(void) {
    _LATA0 = 0;
    _TRISA0 = 1;
    _LATB1 = 0;
    _TRISB1 = 1;

    AD1CON1bits.ADON = 0; // Turn on the A/D converter
    AD1CON1bits.FORM = 0b00; //0;   //INTEGER)
    AD1CON1bits.SSRC = 2; //010 = Timer3 compare ends sampling and starts conversion
    AD1CON1bits.ASAM = 1; //1 = Sampling begins immediately after last conversion. SAMP bit is auto-set
    AD1CON1bits.AD12B = 0; //10 BIT
    AD1CON2bits.VCFG = 0b000; //VDD - VSS,0-3V3
    AD1CON2bits.SMPI = 1; //15;  //ADC interrupt is generated at the completion of every 16st sample/conversion operation
    AD1CON2bits.BUFM = 0; //0 = Always starts filling the buffer from the start address
    AD1CON3bits.ADRC = 0; //0 = Clock Derived from System Clock
    AD1CON3bits.ADCS = 1; // ADC Conversion Clock Tad=Tcy*(ADCS+1)=(1/16M)*2 =125 ns
    // 0-ADC Conversion Time for 10-bit Tc=12*Tad = 1500ns@32Mhz
    // 1-ADC Conversion Time for 12-bit Tc=14*Tad = 1.120us@50mhz
    AD1CON2bits.CHPS = 0b01; //CH0-CH1 simultaneo
    AD1CON1bits.SIMSAM = 1,
            AD1CHS0bits.CH0SA = 0; // MUXA +ve input selection (AN0) for CH0
    AD1CHS0bits.CH0NA = 0; // MUXA -ve input selection (Vref-) for CH0
    AD1CHS123bits.CH123SA = 1; //AN3,MUX1
    AD1CHS123bits.CH123NA = 0;
    // AD1CHS0bits.CH0SB = 1;      // MUXA +ve input selection (AN3) for CH1
    // AD1CHS0bits.CH0NB = 0;      // MUXA -ve input selection (Vref-) for CH1
    AD1CSSL = 0x0000; //porta0 analog. no scanning
    AD1CSSLbits.CSS0 = 1;
    AD1CSSLbits.CSS3 = 1;
    AD1PCFGL = 0xFFFF;
    AD1PCFGLbits.PCFG0 = 0; //AN0
    AD1PCFGLbits.PCFG3 = 0; //AN3

    IFS0bits.AD1IF = 0; // Clear the A/D interrupt flag bit
    IEC0bits.AD1IE = 0; // Enable A/D interrupt

    //Configura Timer 3

    //***   Timer 3  625us

    TMR3 = 0x0000;
    PR3 = SMPERIODO;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;
    //Start Timer 3
    T3CON = 0x8020; //T3 ON ,0.0625us x 256 16us


    AD1CON1bits.ADON = 0; // Turn on the A/D converter


}
//***********************************************************************

BOOL getBufferFull() {

    return bBufferFull;
}
//***************************************************************************

void setBufferFull(BOOL estado) {

    bBufferFull = estado;

}

//*********************************************************************

void startMedidas() {

   // IEC0bits.AD1IE = 1; // Enable ADC
   // AD1CON1bits.ADON = 1;
    IEC1bits.INT1IE  = 1;
    bBufferFull = FALSE;

}
//*********************************************************************

void _ISR __attribute__((__no_auto_psv__)) _ADC1Interrupt(void) {



    inSamplesCurr[adcCntr] = ADC1BUF3; //ReadADC10( 1 );
    inSamplesVolt[adcCntr] = ADC1BUF0; //ReadADC10( 1 );

    /*
    temp = ADC1BUF0;
    if (temp > 512) temp -= 512;
    else if (temp < 512) temp = 512 - temp;


    vinst = (temp * K_2R2 * K_AD) * K_TRAFO * K_DIV;
    voltageAcc += vinst * vinst; //pow(vinst, 2);

    temp = ADC1BUF3;
    if (temp > 512) temp -= 512;
    else if (temp < 512) temp = 512 - temp;

    iinst = (temp * K_2R2 * K_AD) * K_AMP ;
    currentAcc += iinst * iinst;

    powerAcc += iinst * vinst;

    */
    if (++adcCntr >= SMP_BUFFER ) {
        IEC0bits.AD1IE = 0;
        AD1CON1bits.ADON = 0;
        IEC1bits.INT1IE  = 0; // enable INT1
        adcCntr = 0;
        bBufferFull = TRUE;

    }
    IFS0bits.AD1IF = 0;
}
