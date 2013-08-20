#ifndef __MCP6S22_H
#define __MCP6S22_H

#define ADDRCH     0x41
#define ADDRGAIN   0x40

#define SET_PGA_CS         LATBbits.LATB5 = 1;
#define RES_PGA_CS         LATBbits.LATB5 = 0;


void initMCP6S22();
static void WriteReg(BYTE Address, BYTE Data);
void setChanMCP6s92(BYTE chan);
void setGainMCP6S92(BYTE gain);





#endif