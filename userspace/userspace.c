//Braelan Creswell
//Userspace control code
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "userspace.h"

int main ( int argc, char *argv[] )
{
	int lcd = open("/dev/lcd", O_WRONLY);
	if(lcd == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

	ioctl(lcd, LCDIO_CLEAR);
	printf("Clearing LCD\n");
	
	//sz = write(fd, "hello geeks\n", strlen("hello geeks\n")); 

	close(lcd);
	return 0;
}
