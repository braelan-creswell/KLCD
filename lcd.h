//Braelan Creswell
// LCD Driver
#ifndef LCD_H
#define LCD_H

#include <linux/types.h>
#include <asm/ioctl.h>

#define DRIVER_AUTHOR	"Braelan Creswell"
#define DRIVER_DESC	"4bit mode LCD Kernel Driver for RPi"	

#define LCD_D7   26                   // lcd D7 connection

#define LCD_D6   19                   // lcd D6 connection

#define lcd_D5   13                   // lcd D5 connection

#define LCD_D4    6                   // lcd D4 connection

#define LCD_E    27                   // lcd Enable pin

#define LCD_RS   17                   // lcd Register Select pin


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
