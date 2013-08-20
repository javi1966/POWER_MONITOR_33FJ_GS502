/* 
 * File:   LCD.h
 * Author: Javier
 *
 * Created on 13 de mayo de 2013, 20:22
 */

#ifndef LCD_H
#define	LCD_H

#include <p33FJ32GP202.h>
#include "GenericTypeDefs.h"
#include "delay.h"


/* Conector 2x5   1 -> VSS
                  2 -> VDD
                  3 -> VEE
                  4 -> RS
                  5 -> RW
                  6 -> E
                  7 -> D4
                  8 -> D5
                  9 -> D6
                 10 -> D7

 */
#define DB7_TRIS _TRISB13
#define DB6_TRIS _TRISB14
#define DB5_TRIS _TRISB11
#define DB4_TRIS _TRISB12
#define TRIS_RS	 _TRISB9
#define TRIS_E   _TRISB10

#define LCD_RS  _LATB9
//#define LCD_RW  _LATB10
#define LCD_E   _LATB10
#define LCD_DB4  _LATB12
#define LCD_DB5  _LATB11
#define LCD_DB6  _LATB14
#define LCD_DB7  _LATB13

#define LCDDATA 1
#define LCDCMD  0
#define PMDATA  PMDIN
#define busyLCD() readLCD(LCDCMD) & 0x80
#define addrLCD() readLCD(LCDCMD) & 0x7F
#define getLCD()  readLCD(LCDDATA)
#define putLCD(d) writeLCD(LCDDATA,(d))
#define cmdLCD(c) writeLCD(LCDCMD,(c))
#define homeLCD() writeLCD(LCDCMD,2)
#define clrLCD()  writeLCD(LCDCMD,1)
#define setLCDG(a) writeLCD( LCDCMD,(a & 0x3F) | 0x40)
#define setLCDC(a) writeLCD( LCDCMD,(a & 0x7F) | 0x80)




#define LCD_FUNCTION_SET      0x38 // 0b00110000
#define LCD_FUNCTION_SET_4BIT 0x28 // 0b00101000
#define LCD_DISPLAY_OFF       0x08 // 0b00001000
#define LCD_DISPLAY_ON        0x0F // 0b00001111
#define LCD_DISPLAY_CLEAR     0x01 // 0b00000001
#define LCD_ENTRY_MODE_SET    0x06 // 0b00000110
#define LCD_CURSOR_HOME       0x02 // 0b00000010



#define lcd_type         2        // 0=5x7, 1=5x10, 2=2 lines
#define lcd_line_two     0x40 // LCD RAM address for the 2nd line




//Prototipos
void LcdSendNibble( BYTE  );
void LcdSendByte(BYTE );
void writeLCD(BYTE,char);
char readLCD(int );
void lcd_gotoxy(BYTE ,BYTE );
void putsLCD(char *s);
void setCGRAM(BYTE addr,BYTE *bmp);
void drawProgressBar(int index,int imax,int size);
void LcdInitialize(void);



#endif	/* LCD_H */

