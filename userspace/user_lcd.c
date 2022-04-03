//Braelan Creswell
//Userspace control code
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "user_lcd.h"

int main ( int argc, char *argv[] )
{
	
	if (argc < 2) {
		printf("Usage: ./user_lcd <command> <args>\n");
	    return -1;
    }
	
	int lcd = open("/dev/lcd", O_WRONLY);
	if(lcd == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

    switch (atoi(argv[1])) {
    case 2:	
	    ioctl(lcd, LCDIO_CLEAR);
	    printf("Clearing LCD\n");
		break;
    case 1:
	    ioctl(lcd, LCDIO_INIT);
	    printf("Initializing LCD\n");
		break;
	case 3:
	    write(lcd, argv[2], strlen(argv[2]));
	    printf("Writing to LCD\n");
		break;
	case 4:
	    ioctl(lcd, LCDIO_OFF);
	    printf("Turning Off LCD\n");
		break;
	case 5:
	    ioctl(lcd, LCDIO_ON);
	    printf("Turning On LCD\n");
		break;
	case 6:
	    ioctl(lcd, LCDIO_CURSOR1);
	    printf("Setting Cursor 1\n");
		break;
	case 7:
	    ioctl(lcd, LCDIO_CURSOR2);
	    printf("Setting Cursor 2\n");
		break;
	case 8:
	    printf("Testing Locks\n");
		fork();
		write(lcd, "Fork ", strlen("Fork "));
		break;
	default:
	    printf("Command not found\n");
		printf("1: Init LCD\n");
		printf("2: Clear LCD\n");
		printf("3 <string>: Write to LCD\n");
		printf("4: Turn Off LCD\n");
		printf("5: Turn On LCD\n");
		printf("6: Cursor 1st line\n");
		printf("7: Cursor 2nd line\n");
		printf("8: Testing Locks\n");
		break;
	} 

	close(lcd);
	return 0;
}
