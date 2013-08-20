/* 
 * File:   main.c
 * Author: Javier 
 *
 * Created on 22 de abril de 2012, 18:22
 */

#include "p33FJ16GS502.h"
#include <math.h>
#include "adc.h"
#include "delay.h"
#include "uart.h"

#define LED       LATBbits.LATB0  

// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.
#define GetSystemClock()		(32000000ul)
#define GetInstructionClock()	(GetSystemClock()/2)
#define GetPeripheralClock()	(GetSystemClock()/2)



//#define _RELOJ_FRC    //descomentar para reloj FRC y cambiar bajo Config bits

/* Configuration Bit Settings */
_FOSCSEL(FNOSC_PRI)
_FOSC(POSCMD_XT & FCKSM_CSECMD & OSCIOFNC_OFF)
_FWDT(FWDTEN_OFF)
_FPOR(FPWRT_PWR128)
_FICD(ICS_PGD3 & JTAGEN_OFF)


#define T300  30
#define T500  50
#define T1000 100

//**************************************************************
extern WORD inSamplesCurr[SMP_BUFFER];
extern WORD inSamplesVolt[SMP_BUFFER];
BOOL bVisu = FALSE;
extern volatile UINT32 voltageAcc;
extern volatile UINT32 currentAcc;
extern volatile double powerAcc;
BYTE tempo;
//const  char * UART_COMMAND[] = "R02E04X";

int main(int argc, char** argv) {


    char bufferLCD[20];
    char buffer[50];
    UINT32 voltageAccMean = 0;
    UINT32 currentAccMean = 0;
    double voltageRMS = 0.0;
    double corrienteRMS = 0.0;
    double potAparente = 0;
    double potActiva = 0;
    double potReactiva = 0;
    double factorPot = 0;
    BYTE valor;

#ifdef _RELOJ_FRC

    /* Configure Oscillator to operate the device at 40Mhz
      Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
       Fosc= 7.37*(43)/(2*2)=80Mhz for Fosc, Fcy = 40Mhz */

    /* Configure PLL prescaler, PLL postscaler, PLL divisor */
    PLLFBD = 14; /* M = PLLFBD + 2 */
    CLKDIVbits.PLLPOST = 0; /* N1 = 2 */
    CLKDIVbits.PLLPRE = 0; /* N2 = 2 */
    OSCTUN = 22; //8 Mhz
    __builtin_write_OSCCONH(0x01); /* New Oscillator FRC w/ PLL */
    __builtin_write_OSCCONL(0x01); /* Enable Switch */
    while (OSCCONbits.COSC != 0b001); /* Wait for new Oscillator to become FRC w/ PLL */
    while (OSCCONbits.LOCK != 1); /* Wait for Pll to Lock */

    /* Now setup the ADC and PWM clock for 120MHz
       ((FRC * 16) / APSTSCLR ) = (7.37 * 16) / 1 = ~ 120MHz*/


    ACLKCONbits.FRCSEL = 1; // FRC provides input for Auxiliary PLL (x16)
    ACLKCONbits.SELACLK = 1; // Auxiliary Oscillator provides clock source for PWM & ADC
    ACLKCONbits.APSTSCLR = 7; // Divide Auxiliary clock by 1
    ACLKCONbits.ENAPLL = 1; // Enable Auxiliary PLL

    while (ACLKCONbits.APLLCK != 1); // Wait for Auxiliary PLL to Lock

#endif

    LATB = 0;
    TRISB = 0;


    //Funciona Timer1,2,UART y A/D
   
   /* PMD1 = 0xFFFF;
    PMD1bits.ADCMD = 0;
    PMD1bits.T1MD = 0;
    PMD1bits.T2MD = 0;
    PMD1bits.U1MD = 0;
    PMD1bits.T3MD = 0;*/
   // PMD2 = 0xFFFF;
   // PMD3 = 0xFFFF;
  //  PMD4 = 0xFFFF;
  //  PMD6 = 0xFFFF;
  //  PMD7 = 0xFFFF;
    



    initAD();
    LcdInitialize();
    putsLCD("\f**Monitor Potencia**\n");

    /*

    initUART();
    putrsUART("Voltaje;Corriente;Pot.Real;Pot.React;Pot. Apa;Factor Pot\r\n");
    putrsUART("=========================================================\r\n");

   
    
     */
    //1/16000000,62.5ns * 256 =16us
    // Use Timer 1 for 16-bit and 32-bit processors
    // 1:256 prescale
    T1CONbits.TCKPS = 0b11;
    // Base
    PR1 = 625; //aprox 10ms
    // Clear counter
    TMR1 = 0;
    // Enable timer interrupt
    IPC0bits.T1IP = 6; // Interrupt priority 2 (low)
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;





    delay_ms(3000);



    tempo = T300;



    //*****************************************************************
    while (TRUE) {

        putsLCD("\fVolt Corr PoAc PoAp \n");
        putsLCD(bufferLCD);
       

        if (bVisu) {

            //  putsLCD("\fVolt Corr PoAc PoAp \n");

            startMedidas();
            while (!getBufferFull());


            voltageAccMean = voltageAcc / SMP_BUFFER;
            voltageRMS = (double) sqrt(voltageAccMean); //factor de ajuste ????


            currentAccMean = currentAcc / SMP_BUFFER;
            corrienteRMS = (double) sqrt(currentAccMean) * K_CS60;




            potActiva = powerAcc / SMP_BUFFER;
            potAparente = voltageRMS * corrienteRMS;
            factorPot = potActiva / potAparente;
            potReactiva = sqrt((UINT32) (potAparente * potAparente - potActiva * potActiva));




            //   sprintf(buffer, "%3.1f;%2.1f;%4.0f;%4.0f;%4.0f;%1.1f\r\n", voltageRMS, corrienteRMS \
                            , potActiva, potReactiva, potAparente, factorPot);


            // putrsUART(buffer);

            /*
            valor = (corrienteRMS * 100) / 40;
            lcd_gotoxy(1, 1);
            drawProgressBar(valor, 100, 15);
            lcd_gotoxy(17, 1);
            sprintf(bufferLCD, "%2.1f", corrienteRMS);
            putsLCD(bufferLCD);


             valor = (potActiva * 100) / 8000;
              lcd_gotoxy(1, 2);
              drawProgressBar(valor, 100, 15);
               lcd_gotoxy(17, 2);
               sprintf(bufferLCD, "%4.0f", potActiva);
               putsLCD(bufferLCD);*/




            sprintf(bufferLCD, "%3.1f %2.1f %4.0f %4.0f\n", voltageRMS, corrienteRMS, potActiva, potReactiva);
            voltageAcc = 0;
            currentAcc = 0;
            powerAcc = 0;
            LED ^= 1;
            bVisu = FALSE;

        }


       



    }


    return (1);
}


//*****************************************************************
// cada 10ms
void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void) {

    static int count = 0;
    static BYTE ticks = 0;

    if (count++ > 399) {
        bVisu = TRUE;
        count = 0;
    }

    if (ticks++ > tempo) //1 seg
    {
        //   LED ^= TRUE;
        ticks = 0;
    }

    // Reset interrupt flag
    IFS0bits.T1IF = 0;
}

