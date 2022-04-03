//Braelan Creswell
//Userspace control header
#ifndef USERSPACE_H
#define USERSPACE_H

#include <linux/types.h>
#include <asm/ioctl.h>

//IOCTL Commands
#define MAGIC_NUM       'a'

#define LCDIO_INIT      _IO(MAGIC_NUM, 1)
#define LCDIO_CLEAR     _IO(MAGIC_NUM, 2)
#define LCDIO_ON        _IO(MAGIC_NUM, 3)
#define LCDIO_OFF       _IO(MAGIC_NUM, 4)
#define LCDIO_CURSOR1   _IO(MAGIC_NUM, 5)
#define LCDIO_CURSOR2   _IO(MAGIC_NUM, 6)

#endif
