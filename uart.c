#include "uart.h"

void initUART() {

    _TRISB7 = 1;    //RB7 -> RX
    _TRISB6 = 0;    //RB6 -> TX

    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bclr OSCCON, #6");


    RPINR18bits.U1RXR = 7;        //RB7 RX
    RPOR3bits.RP6R = 0b00011;     //RB6 TX

    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bset OSCCON, #6");






    U1MODEbits.UARTEN = 0;
    U1MODEbits.USIDL = 0; // Bit13 Continue in Idle
    U1MODEbits.IREN = 0; // Bit12 No IR translation
    U1MODEbits.RTSMD = 0; // Bit11 Simplex Mode
    U1MODEbits.UEN = 0; // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0; // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0; // Bit6 No Loop Back
    U1MODEbits.ABAUD = 0; // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.URXINV = 0; // Bit4 IdleState = 1  (for dsPIC)
    U1MODEbits.BRGH = 0; // Bit3 16 clocks per bit period
    U1MODEbits.PDSEL = 0; // Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0; // Bit0 One Stop Bit

    // Load a value into Baud Rate Generator.  Example is for 9600.
    // See section 19.3.1 of datasheet.
    //  U1BRG = (Fcy / (16 * BaudRate)) - 1
    //  U1BRG = (36850000 / (16 * 9600)) - 1
    //  U1BRG = 238.908854 //Round to 239

    U1BRG = 103;//103; //9600

    // Load all values in for U1STA SFR
    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0; //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15
    //U2STAbits.notimplemented = 0;	//Bit12
    U1STAbits.UTXBRK = 0; //Bit11 Disabled
    U1STAbits.UTXEN = 0; //Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0; //Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0; //Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0; //Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0; //Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0; //Bit4 *Read Only Bit*
    U1STAbits.PERR = 0; //Bit3 *Read Only Bit*
    U1STAbits.FERR = 0; //Bit2 *Read Only Bit*
    U1STAbits.OERR = 0; //Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0; //Bit0 *Read Only Bit*


    U1MODEbits.UARTEN = 1; // And turn the peripheral on

    U1STAbits.UTXEN = 1;
    // I think I have the thing working now.


}

//***********************************************************************
void putsUART(unsigned int *buffer)
{

  char * temp_ptr = (char *) buffer;

    /* transmit till NULL character is encountered */

    if(U1MODEbits.PDSEL == 3)        /* check if TX is 8bits or 9bits */
    {
        while(*buffer != '\0')
        {
            while(U1STAbits.UTXBF); /* wait if the buffer is full */
            U1TXREG = *buffer++;    /* transfer data word to TX reg */
        }
    }
    else
    {
        while(*temp_ptr != '\0')
        {
            while(U1STAbits.UTXBF);  /* wait if the buffer is full */
            U1TXREG = *temp_ptr++;   /* transfer data byte to TX reg */
        }
    }
}

//**************************************************************************
unsigned int getsUART1(unsigned int length,unsigned int *buffer,
                       unsigned int uart_data_wait)

{
    unsigned int wait = 0;
    char *temp_ptr = (char *) buffer;

    while(length)                         /* read till length is 0 */
    {
        while(!DataRdyUART())
        {
            if(wait < uart_data_wait)
                wait++ ;                  /*wait for more data */
            else
                return(length);           /*Time out- Return words/bytes to be read */
        }
        wait=0;
        if(U1MODEbits.PDSEL == 3)         /* check if TX/RX is 8bits or 9bits */
            *buffer++ = U1RXREG;          /* data word from HW buffer to SW buffer */
		else
            *temp_ptr++ = U1RXREG & 0xFF; /* data byte from HW buffer to SW buffer */

        length--;
    }

    return(length);                       /* number of data yet to be received i.e.,0 */
}

//******************************************************************************
char DataRdyUART(void)
{
    return(U1STAbits.URXDA);
}
//******************************************************************************
char BusyUART(void)
{
    return(!U1STAbits.TRMT);
}
//***************************************************************************
unsigned int ReadUART(void)
{
    if(U1MODEbits.PDSEL == 3)
        return (U1RXREG);
    else
        return (U1RXREG & 0xFF);
}
//***************************************************************************
void WriteUART(unsigned int data)
{
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}







