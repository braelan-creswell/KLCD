//Braelan Creswell
// LCD Driver
#ifndef LCD_H
#define LCD_H

#include <linux/types.h>
#include <asm/ioctl.h>

//IOCTL Commands
#define MAGIC_NUM       ???

#define LCDIO_WRITE     _IO(MAGIC_NUM, 1)
#define LCDIO_CLEAR     _IO(MAGIC_NUM, 2)
#define LCDIO_COMMAND   _IO(MAGIC_NUM, 3)

// LCD module information
#define LCD_LINEONE     0x00                    //line 1
#define LCD_LINETWO     0x40                    //line 2


// LCD instructions
#define LCD_CLEAR          0b00000001          // replace all characters with space
#define LCD_HOME           0b00000010          // return cursor to first position
#define LCD_ENTRYMODE      0b00000110          
#define LCD_DISPLAYOFF     0b00001000          // turn display off
#define LCD_DISPLAYON      0b00001111          // display on, cursor on, blink character
#define LCD_FUNCTIONRESET  0b00110000          // reset
#define LCD_FUNCTIONSET    0b00101000          // 4-bit data, 2-line display, 5 x 7 font
#define LCD_SETCURSOR      0b10000000          // set cursor position

#endif	// LCD_H
