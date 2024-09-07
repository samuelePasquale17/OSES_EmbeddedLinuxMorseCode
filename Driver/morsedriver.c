#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/uaccess.h> /* copy_to copy_from _user */
#include <asm/io.h>
#include "morsedriver.h"

#define SUCCESS 0
#define ERROR -1
#define DEVICE_NAME "/dev/morse_Dev"
#define BASE_ADDRESS 0x43C00000
#define LEN_BUFFER_KERNEL 63
#define ENABLE_OFFSET_REG 63
#define ENABLE_CONVERSION 1
#define DEBUG

static unsigned long *mmio;
static int major_num;
int device_max_size = DEVICE_MAX_SIZE;

/* Prevent concurent access into the same device */
static int Device_Open = 0;

/* prototipes */
void write_charatter(char c, int i);

/* Called whenever a process attempts to open the device file */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "[MORSE DRIVER] device_open(%p)\n", file); /* debug message */
#endif

	/* Only one processes at the same time */
	if (Device_Open)
		return -EBUSY;
	Device_Open++;

	/* Initialize the message */
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "[MORSE DRIVER] device_release(%p,%p)\n", inode, file); /* debug message */
#endif

	/* Ready for the next caller */
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}

/* Called whenever a process which has already opened the device file attempts to read from it */
static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
	return SUCCESS;
}

/* Called when somebody tries to write into the device file */
static ssize_t device_write(struct file *file,
							const char __user *buffer, /* user buffer */
							size_t length,			   /* number of integers*/
							loff_t *offset)
{
	int retval = 0, i = 0;
	char *bufferKernel = NULL; /* buffer in kernel space */

	if (length > LEN_BUFFER_KERNEL) /* check size */
	{
#ifdef DEBUG
		printk("[MORSE DRIVER] device_write() : trying to write more than possible. Aborting write \n"); /* debug message */
#endif

		retval = -EFBIG;
		return retval;
	}

	/* buffer in kernel space */
	bufferKernel = kmalloc(sizeof(char) * length, GFP_KERNEL);

	if (!bufferKernel)
	{ /*check kmalloc */
#ifdef DEBUG
		printk("[MORSE DRIVER] device_write() : error kmalloc. Aborting write \n"); /* debug message */
#endif
		if (bufferKernel != NULL)
			kfree(bufferKernel); /* free memory */
		return ERROR;
	}

	if (copy_from_user(bufferKernel, buffer, sizeof(char) * length)) /* copy buffer into kernel space */
	{
#ifdef DEBUG
		printk("[MORSE DRIVER] device_write() : can't use copy_from_user.  \n"); /* debug message */
#endif

		if (bufferKernel != NULL)
			kfree(bufferKernel); /* free memory */
		retval = -EPERM;
		return retval;
	}
	else
	{
		for (i = 0; i < length; i++)
		{										 /* scan string */
			write_charatter(bufferKernel[i], i); /* write character into memory location */

#ifdef DEBUG
			printk("[MORSE DRIVER] device_write() : writing %c ...\n", bufferKernel[i]); /* debug message */
#endif
		}
	}
	return retval;
}

/* write character */
void write_charatter(char c, int i)
{
	if ((c >= 65 && c <= 90) || (c == 32)) /* It is a alph letter */
	{
		*(unsigned int *)(mmio + i) = (unsigned int)c;
	}
	else if ((c >= 97 && c <= 122)) /* It is a alph letter */
	{
		*(unsigned int *)(mmio + i) = (unsigned int)(c - 32);
	}
	return;
}

/* Called whenever a process tries to do an ioctl on our device file */
long device_ioctl(struct file *file,
				  unsigned int ioctl_num, /* number and param for ioctl */
				  unsigned long ioctl_param)
{
	if (ioctl_num == ENABLE_CONVERSION)
	{
		/* start conversion */
		*(unsigned int *)(mmio + ENABLE_OFFSET_REG) = (unsigned int)2; /* enable conversion writing onto the second bit of reg63 */
	}
	return SUCCESS;
}

/* Module Declarations */
/* This structure will hold the functions to be called when a process does something to the device we
 * created. The pointer to this structure is kept in the devices table */
struct file_operations Fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release, /*close */
};

/* Module information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 1 - Politecnico di Torino");
MODULE_DESCRIPTION("Morse - Loadable Kernel Module for morse application");

#define DRIVER_NAME "morsedriver" /* driver name */

static struct of_device_id morse_of_match[] = {
	{
		.compatible = "xlnx,axi4_converter_v1_0",
	},
	{/* end of list */},
};
MODULE_DEVICE_TABLE(of, morse_of_match);

static struct platform_driver morse_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = morse_of_match,
	},
};

static int __init morse_init(void)
{
	int rc = 0;
#ifdef DEBUG
	printk("[MORSE DRIVER] Hello from MORSE DRIVER.\n"); /* debug message */
#endif

	/* Register the character device */
	major_num = register_chrdev(0, DEVICE_NAME, &Fops);

	/* Negative values signify an error */
	if (major_num < 0)
	{
#ifdef DEBUG
		printk(KERN_ALERT "[MORSE DRIVER] %s failed with \n", "Sorry, registering the character device "); /* debug message */
#endif
	}

	mmio = ioremap(BASE_ADDRESS, 0x100); /* map physical <-> logical memory */

#ifdef DEBUG
	printk("[MORSE DRIVER] Registers mapped \n");
	printk(KERN_INFO "[MORSE DRIVER]  %s the major device number is %d.\n", "Registeration is a success", major_num); /* debug message */
#endif

	rc = platform_driver_register(&morse_driver); /* Register driver */

	return rc;
}

static void __exit morse_exit(void)
{
	unregister_chrdev(major_num, DEVICE_NAME);
	platform_driver_unregister(&morse_driver);
	printk(KERN_ALERT "Goodbye module world.\n");
}

module_init(morse_init);
module_exit(morse_exit);
