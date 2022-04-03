#ECE 331 S22 Kernel Driver RPi v4, Linux Kernel 
##Braelan Creswell
###Operation
Wire LCD as Follows:
| LCD PIN | GPIO PIN | Pi PIN |
| ------- | -------- | ------ |
| LCD RS  | GPIO 17  | J8-11  |
| LCD E   | GPIO 27  | J8-13  |
| DATA D4 | GPIO 6   | J8-31  |
| DATA D5 | GPIO 13  | J8-33  |
| DATA D6 | GPIO 19  | J8-35  |
| DATA D7 | GPIO 26  | J8-37  |
| +5V     | +5V      | J8-4   |
| GND     | GND      | J8-6   |

Build module: make
Insert module: sudo insmod lcd.ko
Build userspace program: gcc user_lcd.c -o user_lcd
Run userspace program: ./user_lcd <command> <args>

###Commands
1: Init LCD
2: Clear LCD
3 <string>: Write to LCD
4: Turn Off LCD
5: Turn On LCD
6: Cursor 1st line
7: Cursor 2nd line
8: Test Locks

