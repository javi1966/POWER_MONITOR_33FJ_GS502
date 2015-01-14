
#include <p33FJ32GP202.h>
#include <math.h>
#include <stdio.h>
#include "adc.h"
#include "LCD.h"
#include "uart.h"

_FOSCSEL(FNOSC_PRI) //Oscilador Fast RC 8Mhz
_FOSC(POSCMD_HS & IOL1WAY_OFF & OSCIOFNC_OFF)
_FWDT(FWDTEN_OFF)
//_FICD(ICS_PGD1 & JTAGEN_OFF)

#define FCY (32000000/2)  // 0.0625 us,16 MIPS
#define GetSystemClock()            32000000UL
#define GetPeripheralClock()        (GetSystemClock())
#define GetInstructionClock()       (GetSystemClock() / 2)

#define LED  _LATB0

//para delay's
#define TMRX       TMR2
#define TXCON      T2CON
#define DELAY1US   16       //32Mhz @ TCY=0.0625*16=1us
#define DELAY1MS   16000    //idem

BOOL bVisu = FALSE;
extern volatile UINT32 voltageAcc;
extern volatile UINT32 currentAcc;
extern volatile double powerAcc;
/*
//*******************************************************************************************************
// Delays
//*******************************************************************************************************

void delay_us(unsigned int valor) {
    unsigned int i = 0;
    TXCON = 0x8000; //arranca timero sin divisor.
    TMRX = 0x0000; //inicializa a 0

    while (i < valor) {
        while (TMRX < DELAY1US);
        i++;
        TMRX = 0x0000;
    }
}
//*********************************************************************************************************

void delay_ms(unsigned int valor) {
    unsigned int i = 0;
    TXCON = 0x8000; //arranca timero sin divisor.
    TMRX = 0; //inicializa a 0

    while (i < valor) {
        while (TMRX < DELAY1MS);
        i++;
        TMRX = 0;
    }
}
 */

//*******************************************************************************************************

int writeSPI(int valor) {
    SPI1BUF = valor;
    while (!SPI1STATbits.SPIRBF);
    return SPI1BUF;
}

//**********************************************************************************************************

int main(void) {


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

    /*******************************************************
      //Setup Oscilador
      //fosc=fin*(M/N1*N2)
      //Fosc =32MHZ , Fin FRC con 8MHz (OSCTUN=22)
      // 32 = 8 *( 16 / 2 * 2)
      // 50 = 8 *(25 /2*2)
      PLLFBD = 14; // factor M=16(32Mhz) ,M=25 (50Mhz) ,se resta 2 ,empieza con 2
      CLKDIVbits.PLLPOST = 0; // divisor por 2
      CLKDIVbits.PLLPRE = 0; // divisor por 2
      OSCTUN = 22; // ajuste fino FRC a 8Mhz
      while (OSCCONbits.LOCK == 0); // espera enganche PLL

     */

    //configura puertos
    LATA = 0;
    LATB = 0;
    TRISB = 0x0000;
    TRISA = 0x0000;


    /*
    //Configura pines para el  SPI1
    // /CS -> RB7 , SCK -> RP8 , SDO -> RP9,ver tabla 9-3 del DataSheet
     RPOR4bits.RP8R=8;    // SCK1OUT
     RPOR4bits.RP9R=7;    // SDO1
     __builtin_write_OSCCONL(OSCCON | 0x40);  //pone a set IOCLOCK
     */
    /*
    //Configura TMR1
     TMR1=0;
     PR1=6250;              // 100.000 us/FCY * 256(Preescaler)
     IFS0bits.T1IF=0;
     IEC0bits.T1IE=1;       //una int cada 100ms
     T1CON=0x8030;         //T1 ON,divisor 256
     */
    //*************************************************


    // Use Timer 1 for 16-bit and 32-bit processors
    // 1:256 prescale
    T1CONbits.TCKPS = 3;
    // Base
    PR1 = 0xFFFF;
    // Clear counter
    TMR1 = 0;

    // Enable timer interrupt

    IPC0bits.T1IP = 2; // Interrupt priority 2 (low)
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;

    // Start timer
    T1CONbits.TON = 1;

    //INT1 sobre RB8
    _TRISB8 = 1;    //RB7 -> RX
    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bclr OSCCON, #6");


    RPINR0bits.INT1R = 8;        //RB8 INT1

    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bset OSCCON, #6");

    IFS1bits.INT1IF  = 0;
    IEC1bits.INT1IE  = 0; // enable INT1


    initAD();
    LcdInitialize();
    putsLCD("\f**Monitor Potencia**\n");
    initUART();
    putrsUART("Voltaje;Corriente;Pot.Real;Pot.React;Pot. Apa;Factor Pot\r\n");
    putrsUART("=========================================================\r\n");

    while (1) {

        if (bVisu) {

            // putsLCD("\fVolt Corr PoAc PoAp \n");


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


            sprintf(buffer, "%3.1f;%2.1f;%4.0f;%4.0f;%4.0f;%1.1f\r\n", voltageRMS, corrienteRMS \
                    , potActiva, potReactiva, potAparente, factorPot);

            putrsUART(buffer);

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
            putsLCD(bufferLCD);

            // sprintf(bufferLCD,"%3.1f %2.1f %4.0f %4.0f\r\n",voltageRMS,corrienteRMS,potActiva,potReactiva);
            // putsLCD(bufferLCD);

            voltageAcc = 0;
            currentAcc = 0;
            powerAcc = 0;
            LED ^= 1;
            bVisu = FALSE;

        }


        /* if (getBufferFull()) {
             IEC0bits.AD1IE = 1;
             AD1CON1bits.ADON = 1;
             setBufferFull(FALSE);*/

    }


}


//Interrupciones
//********************************************************
void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void) {

    static BYTE count = 0;

    //if (count++ > 99)
    bVisu = TRUE;

    // Reset interrupt flag
    IFS0bits.T1IF = 0;
}

//******************************************************
void __attribute__((__interrupt__)) _INT1Interrupt(void)
{
   IEC0bits.AD1IE = 1;
   AD1CON1bits.ADON = 1;
   TMR3 = 0x0000;
   IEC1bits.INT1IE  = 0;
   IFS1bits.INT1IF  = 0; // Reset Flag interrupt INT1
}