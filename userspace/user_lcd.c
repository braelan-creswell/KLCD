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
		printf("Usage: ./user_lcd <command> <args>\n"); //Make sure there are atleast 2 arguments to the program
	    return -1;
    }
	
	int lcd = open("/dev/lcd", O_WRONLY); //Open device file
	if(lcd == -1) {                       //Check for errors opening file
		printf("Opening was not possible!\n"); 
		return -1;
	}

    switch (atoi(argv[1])) { //Use first CL argument as command switch
    case 2:	
	    ioctl(lcd, LCDIO_CLEAR); //Clear LCD IOCTL
	    printf("Clearing LCD\n");
		break;
    case 1:
	    ioctl(lcd, LCDIO_INIT); //INIT LCD IOCTL
	    printf("Initializing LCD\n");
		break;
	case 3:
	    write(lcd, argv[2], strlen(argv[2])); //Write to display using 2nd CL argument as data
	    printf("Writing to LCD\n");
		break;
	case 4:
	    ioctl(lcd, LCDIO_OFF); //LCD Off IOCTL
	    printf("Turning Off LCD\n");
		break;
	case 5:
	    ioctl(lcd, LCDIO_ON); //LCD On IOCTL
	    printf("Turning On LCD\n");
		break;
	case 6:
	    ioctl(lcd, LCDIO_CURSOR1); //Place Cursor on 1st line IOCTL
	    printf("Setting Cursor 1\n");
		break;
	case 7:
	    ioctl(lcd, LCDIO_CURSOR2); //Place Cursor on 2nd line IOCTL
	    printf("Setting Cursor 2\n");
		break;
	case 8:
	    printf("Testing Locks\n"); //Test locking in kernel driver
		fork(); //fork process
		write(lcd, "Fork ", strlen("Fork ")); //both parent and child process send write to kernel driver, proper function is 2 "Fork " displayed on screen
		break;
	default:
	    printf("Command not found\n"); //Give user list of commands if incorrect command entered
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
