//Braelan Creswell
//Userspace control header
#ifndef USERSPACE_H
#define USERSPACE_H

//IOCTL Commands
#define MAGIC_NUM       0xA

#define LCDIO_WRITE     _IO(MAGIC_NUM, 1)
#define LCDIO_CLEAR     _IO(MAGIC_NUM, 2)
#define LCDIO_COMMAND   _IOW(MAGIC_NUM, 3, int)

#endif
