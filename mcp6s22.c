#include "p33FJ16GS502.h"
#include "GenericTypeDefs.h"
#include "mcp6s22.h"



static unsigned char gucPGAGain;

 #define WRITEPGA(cmd,val,dummy)\
      {\
         RES_PGA_CS;\
         SPI1BUF = (unsigned int) ((((unsigned int)(cmd))<<8) | (val));\
         while (!SPI1STATbits.SPIRBF)\
         {\
            ;\
         }\
         (dummy) = SPI1BUF; /* Dummy read to clear the SPIRBF flag */\
         SET_PGA_CS;\
      }
#define ClearSPIDoneFlag()
static inline __attribute__((__always_inline__)) void WaitForDataByte( void )
    {
        while ((SPI1STATbits.SPITBF == 1) );
}

void initMCP6S22()
{
    //****** Configura SPI para MCP6S22
    _TRISB6 = 0;
    _TRISB7 = 0;
    _TRISB5 = 0;  //CS
    __builtin_write_OSCCONL(OSCCON & 0xDF);        // to clear IOLOCK

    RPOR3bits.RP6R=0x08;    // RP6->SCK1OUT
    RPOR3bits.RP7R=0x07;    // RP7->SDO1
     __builtin_write_OSCCONL(OSCCON | 0x40);

    SPI1STAT = 0;
   // SPI1CON1 = 0x0F;  // 1:1 primary prescale, 5:1 secondary prescale (8MHz  @ 40MIPS)
    SPI1CON1bits.PPRE = 2;   /* Primary prescaler set to 4 */
    SPI1CON1bits.SPRE = 4;   /* Secondary prescaler set to 4 */
    SPI1CON2 = 0;
    SPI1CON1bits.MODE16 = 1;
    SPI1CON1bits.CKE = 1;
    SPI1CON1bits.MSTEN = 1;
    SPI1STATbits.SPIEN = 1;




}

//**************************************************************************
static void WriteReg(BYTE Address, BYTE Data)
{
    volatile BYTE Dummy;

     RES_PGA_CS;
     SPI1BUF = ( Address<<8 ) | (WORD)Data;
     while (!SPI1STATbits.SPIRBF);          // Wait until opcode/constant is transmitted.
     Dummy = SPI1BUF;
     SET_PGA_CS;
}
//***************************************************************************
 void setChanMCP6s92(BYTE chan)
  {
   
     BYTE dummy;
     WriteReg(ADDRCH,chan);
    // WRITEPGA(ADDRCH,chan,dummy);
  }

//***************************************************************************

  void setGainMCP6S92(BYTE gain)
  {
    BYTE code = 0;
    BYTE dummy;

    switch (gain)
    {

      case 1:  code = 0;  break;
      case 2:  code = 1;  break;
      case 4:  code = 2;  break;
      case 5:  code = 3;  break;
      case 8:  code = 4;  break;
      case 10: code = 5;  break;
      case 16: code = 6;  break;
      case 32: code = 7;  break;
      default: return -1;
    }

     WriteReg(ADDRGAIN,gain);
 //  WRITEPGA(ADDRGAIN,gain,dummy);

  }

  /* ----------------------------------------------------------- */

void PGA_IncGain(void)
{
   unsigned char ucDummy;

   /* No more gain available */
   if (gucPGAGain == PGAMAXGAIN)
      return;

   /* Increase current gain */
   gucPGAGain++;
   if (gucPGAGain > PGAMAXGAIN)
      gucPGAGain = PGAMAXGAIN;

   /* Send to the PGA */
   PGA_TX_DATA(PGA_SETGAINCMD, gucPGAGain & 0x07, ucDummy);
}

/* ----------------------------------------------------------- */

void PGA_DecGain(void)
{
   unsigned char ucDummy;

   /* No more gain available */
   if (gucPGAGain == PGAMINGAIN)
      return;

   /* Decrease current gain */
   gucPGAGain--;
   if (gucPGAGain == 0xFF)
      gucPGAGain = 0;

   /* Send to the PGA */
   PGA_TX_DATA(PGA_SETGAINCMD, gucPGAGain & 0x07, ucDummy);
}

/* ----------------------------------------------------------- */

unsigned char PGA_GetCurGain()
{
   return gucPGAGain;
}

