// A. Sheaff 2/22/2022 - Framework code
// Braelan Creswell - Kernel Driver
// Operates a 4bit LCD
// Allocate GPIO pins from the kernel
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/jiffies.h>

#include "lcd.h"

// Data to be "passed" around to various functions
struct lcd_data_t {
	struct gpio_desc *gpio_lcd_rs;		// LCD RS pin
	struct gpio_desc *gpio_lcd_e;		// LCD E pin
	struct gpio_desc *gpio_lcd_d4;		// LCD D4 pin
	struct gpio_desc *gpio_lcd_d5;		// LCD D5 pin
	struct gpio_desc *gpio_lcd_d6;		// LCD D6 pin
	struct gpio_desc *gpio_lcd_d7;		// LCD D7 pin
	struct class *lcd_class;			// Class for auto /dev population
	struct device *lcd_dev;				// Device for auto /dev population
	struct platform_device *pdev;		// Platform driver
	int lcd_major;						// Device major number
	//Spinlock locking variable declaration
	spinlock_t lcd_spinlock;
	//spin_lock(&lcd_dat->lcd_spinlock);

};

// LCD data structure access between functions
static struct lcd_data_t *lcd_dat=NULL;

//LCD Initialize Function
static void lcd_init(void) {
	msleep(15); //ensure screen is powered up for long enough
	lcd_sendbits(0, LCD_FUNCTIONRESET); //sends first 4 bits of 8 bit data
	msleep(5);
	lcd_sendbits(0, LCD_FUNCTIONRESET);
	msleep(1);
	lcd_sendbits(0, LCD_FUNCTIONRESET);
	msleep(1);
	lcd_sendbits(0, LCD_FUNCTIONSET);
	msleep(1);
	lcd_sendbyte(0, LCD_FUNCTIONSET); //sends all 8 bits of 8bit data, 4bits at a time
	msleep(1);
	lcd_sendbyte(0, LCD_CLEAR);
	msleep(1);
	lcd_sendbyte(0, LCD_ENTRYMODE);
	msleep(1);
	lcd_sendbyte(0, LCD_DISPLAYON);
}

//Clock Function (Enable Pin)
static void lcd_clk(void) {
	gpiod_set_value(lcd_dat->gpio_lcd_e, 1);
	msleep(5);
	gpiod_set_value(lcd_dat->gpio_lcd_e, 0);
}

//Send bits to LCD
static void lcd_sendbits(int RS, uint8_t data) {
    if(RS)
		gpiod_set_value(lcd_dat->gpio_lcd_rs, 1); //Set register to Data mode
	else
		gpiod_set_value(lcd_dat->gpio_lcd_rs, 0); //Set register to Command mode
		

	gpiod_set_value(lcd_dat->gpio_lcd_e, 0); //Enable pin init low

	gpiod_set_value(lcd_dat->gpio_lcd_d7, 0);
	if(data & 1<<7) gpiod_set_value(lcd_dat->gpio_lcd_d7, 1); //Check 7th bit
	gpiod_set_value(lcd_dat->gpio_lcd_d6, 0);;
	if(data & 1<<6) gpiod_set_value(lcd_dat->gpio_lcd_d6, 1); //check 6th bit
	gpiod_set_value(lcd_dat->gpio_lcd_d5, 0);
	if(data & 1<<5) gpiod_set_value(lcd_dat->gpio_lcd_d5, 1); //check 5th bit
	gpiod_set_value(lcd_dat->gpio_lcd_d4, 0);
	if(data & 1<<4) gpiod_set_value(lcd_dat->gpio_lcd_d4, 1); //check 4th bit

	lcd_clk(); //flip enable pin to write
}

static void lcd_sendbyte(int flag, uint8_t data) {
	lcd_sendbits(flag, data);
	lcd_sendbits(flag, data << 4);
	
}

// ioctl system call
// If an LCD instruction is the command then
//   use arg directly as the value (8 bits) and write to the LCD as an instruction
// Determine if locking is needed and add appropriate code as needed.
static long lcd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int *userbits;
	
	switch (cmd) {
	case LCDIO_INIT:
		printk("Initializing Screen");
		lcd_init();                           //Initialize Display
		break;
	case LCDIO_CLEAR:
		printk("Clearing LCD");
		lcd_sendbyte(0, LCD_CLEAR);           //Send clear command
		break;
	case LCDIO_COMMAND:
	    spin_lock(&lcd_dat->lcd_spinlock);
        if(copy_from_user(&userbits, (int *) arg, sizeof(userbits))) { //copy data from user
		        spin_unlock(&lcd_dat->lcd_spinlock);                   //Unlock before return
				printk("LCD - Error copying data from user!\n");       //handle errors
				return -EFAULT;
		}
		else {
			spin_unlock(&lcd_dat->lcd_spinlock);                     //Unlock before sending data to LCD
			printk("Running LCD Command");
			lcd_sendbyte(0, *userbits);                               //send command to LCD
		}
		break;
	default:
		return -EINVAL;
		break;
	}

	return 0;
}

// Write system call - always check for and act appropriately upon error.
//  DO not return upon while holding a lock
// maximum write size to the LCD is 32 bytes.
//   Do not allow any more.  Return the minimum of count or 32, or an error.
// allocate memory in the kernel for data passed from user space
// copy the data
// Write up to the first 32 bytes of data to the LCD at the proper location
// release the lock
// Free memory
// This function return the number of byte successfully writen
static ssize_t lcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    int i;
	uint8_t not_copied;
	char *data;
	
	if(count > 32) {
		printk("More than 32 bytes sent");
		return -ENOMEM;
	}
	
	spin_lock(&lcd_dat->lcd_spinlock); //Lock
	
	data = kmalloc(count, GFP_KERNEL);
	// Return error if can't allocate memory
	if (data == NULL) {
		spin_unlock(&lcd_dat->lcd_spinlock); //Unlock before return
		printk("Not Enough Memory");
		return -ENOMEM;
	}

	not_copied = copy_from_user(data, buf, count);
	// If there was an error copying from user space return EFAULT
	if (not_copied == 0) {
		printk("Copied %zu bytes", count);
	} else {
		printk("While Copying, %d bytes failed", not_copied);
		kfree(data);
		spin_unlock(&lcd_dat->lcd_spinlock); //Unlock before return
		return -EFAULT;
	}
	
	spin_unlock(&lcd_dat->lcd_spinlock); //Unlock before sending data to LCD
	
	for (i = 0; i < count; i++) {
	    lcd_sendbyte(1, data[i]); //Send data to LCD 8 bits at a time.
	}
    //    while (data[i] != 0) Alternative
	
	kfree(data);
    return count;
}

// Open system call
// Open only if the file access flags are appropiate
static int lcd_open(struct inode *inode, struct file *filp)
{
	//Check flags to make sure  file is write only
	if(!(filp->f_flags & O_WRONLY)){
		printk(KERN_INFO "File is not write only!\n");
		return -1;

	}
	printk(KERN_INFO "File open succeeded!\n");
	filp->private_data=lcd_dat; 

	return 0;
}

// Close system call
static int lcd_release(struct inode *inode, struct file *filp)
{
    printk("Close was called!\n");
	return 0;
}

// File operations for the lcd device.  Uninitialized will be NULL.
static const struct file_operations lcd_fops = {
	.owner = THIS_MODULE,	// Us
	.open = lcd_open,		// Open
	.release = lcd_release,// Close
	.write = lcd_write,	// Write
	.unlocked_ioctl=lcd_ioctl,	// ioctl
};

// Init value<0 means input
static struct gpio_desc *lcd_obtain_pin(struct device *dev, int pin, char *name, int init_val)
{
	struct gpio_desc *gpiod_pin=NULL;	// GPIO Descriptor for setting value
	int ret=-1;	// Return value

	// Request the pin - release with devm_gpio_free() by pin number
	if (init_val>=0) {
		ret=devm_gpio_request_one(dev,pin,GPIOF_OUT_INIT_LOW,name);
		if (ret<0) {
			dev_err(dev,"Cannot get %s gpio pin\n",name);
			gpiod_pin=NULL;
			goto fail;
		}
	} else {
		ret=devm_gpio_request_one(dev,pin,GPIOF_IN,name);
		if (ret<0) {
			dev_err(dev,"Cannot get %s gpio pin\n",name);
			gpiod_pin=NULL;
			goto fail;
		}
	}

	// Get the gpiod pin struct
	gpiod_pin=gpio_to_desc(pin);
	if (gpiod_pin==NULL) {
		printk(KERN_INFO "Failed to acquire lcd gpio\n");
		gpiod_pin=NULL;
		goto fail;
	}

	// Make sure the pin is set correctly
	if (init_val>=0) gpiod_set_value(gpiod_pin,init_val);

	return gpiod_pin;

fail:
	if (ret>=0) devm_gpio_free(dev,pin);

	return gpiod_pin;
}


// Sets device node permission on the /dev device special file
static char *lcd_devnode(struct device *dev, umode_t *mode)
{
	if (mode) *mode = 0666;
	return NULL;
}

// This is called on module load.
static int __init lcd_probe(void)
{
	int ret=-1;	// Return value

	// Allocate device driver data and save
	lcd_dat=kmalloc(sizeof(struct lcd_data_t),GFP_KERNEL);
	if (lcd_dat==NULL) {
		printk(KERN_INFO "Memory allocation failed\n");
		return -ENOMEM;
	}

	memset(lcd_dat,0,sizeof(struct lcd_data_t));

	// Create the device - automagically assign a major number
	lcd_dat->lcd_major=register_chrdev(0,"lcd",&lcd_fops);
	if (lcd_dat->lcd_major<0) {
		printk(KERN_INFO "Failed to register character device\n");
		ret=lcd_dat->lcd_major;
		goto fail;
	}

	// Create a class instance
	lcd_dat->lcd_class=class_create(THIS_MODULE, "lcd_class");
	if (IS_ERR(lcd_dat->lcd_class)) {
		printk(KERN_INFO "Failed to create class\n");
		ret=PTR_ERR(lcd_dat->lcd_class);
		goto fail;
	}

	// Setup the device so the device special file is created with 0666 perms
	lcd_dat->lcd_class->devnode=lcd_devnode;
	//Call device_create
	lcd_dat->lcd_dev=device_create(lcd_dat->lcd_class,NULL,MKDEV(lcd_dat->lcd_major,0),(void *)lcd_dat,"lcd");
	if (IS_ERR(lcd_dat->lcd_dev)) {
		printk(KERN_INFO "Failed to create device file\n");
		ret=PTR_ERR(lcd_dat->lcd_dev);
		goto fail;
	}

	lcd_dat->gpio_lcd_rs=lcd_obtain_pin(lcd_dat->lcd_dev,17,"LCD_RS",0);
	if (lcd_dat->gpio_lcd_rs==NULL) goto fail;
	lcd_dat->gpio_lcd_e=lcd_obtain_pin(lcd_dat->lcd_dev,27,"LCD_E",0);
	if (lcd_dat->gpio_lcd_e==NULL) goto fail;
	lcd_dat->gpio_lcd_d4=lcd_obtain_pin(lcd_dat->lcd_dev,6,"LCD_D4",0);
	if (lcd_dat->gpio_lcd_d4==NULL) goto fail;
	lcd_dat->gpio_lcd_d5=lcd_obtain_pin(lcd_dat->lcd_dev,13,"LCD_D5",0);
	if (lcd_dat->gpio_lcd_d5==NULL) goto fail;
	lcd_dat->gpio_lcd_d6=lcd_obtain_pin(lcd_dat->lcd_dev,19,"LCD_D5",0);
	if (lcd_dat->gpio_lcd_d6==NULL) goto fail;
	lcd_dat->gpio_lcd_d7=lcd_obtain_pin(lcd_dat->lcd_dev,26,"LCD_D7",0);
	if (lcd_dat->gpio_lcd_d7==NULL) goto fail;

    //Initialize Spinlock
	spin_lock_init(&lcd_dat->lcd_spinlock);

	//Initialize LCD
	lcd_init();

/*****************************************************************************/

	printk(KERN_INFO "LCD Registered\n");

	return 0;

fail:
	if (lcd_dat->gpio_lcd_d7) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d7));
	if (lcd_dat->gpio_lcd_d6) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d6));
	if (lcd_dat->gpio_lcd_d5) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d5));
	if (lcd_dat->gpio_lcd_d4) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d4));
	if (lcd_dat->gpio_lcd_e) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_e));
	if (lcd_dat->gpio_lcd_rs) devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_rs));

	// Device cleanup
	if (lcd_dat->lcd_dev) device_destroy(lcd_dat->lcd_class,MKDEV(lcd_dat->lcd_major,0));
	// Class cleanup
	if (lcd_dat->lcd_class) class_destroy(lcd_dat->lcd_class);
	// char dev clean up
	if (lcd_dat->lcd_major) unregister_chrdev(lcd_dat->lcd_major,"lcd");

	if (lcd_dat!=NULL) kfree(lcd_dat);
	printk(KERN_INFO "LCD Failed\n");
	return ret;
}

// Called when the module is removed
static void __exit lcd_remove(void)
{
	// Free the gpio pins with devm_gpio_free() & gpiod_put()
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d7));
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d6));
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d5));
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_d4));
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_e));
	devm_gpio_free(lcd_dat->lcd_dev,desc_to_gpio(lcd_dat->gpio_lcd_rs));

	// Device cleanup
	device_destroy(lcd_dat->lcd_class,MKDEV(lcd_dat->lcd_major,0));
	// Class cleanup
	class_destroy(lcd_dat->lcd_class);
	// Remove char dev
	unregister_chrdev(lcd_dat->lcd_major,"lcd");

	// Free the device driver data
	if (lcd_dat!=NULL) {
		kfree(lcd_dat);
		lcd_dat=NULL;
	}

	printk(KERN_INFO "LCD Removed\n");
}

module_init(lcd_probe);
module_exit(lcd_remove);

MODULE_AUTHOR("Braelan Creswell");
MODULE_DESCRIPTION("4bit mode LCD Kernel Driver for RPi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KLCD");
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include "lcd.h"