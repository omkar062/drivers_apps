#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <asm/uaccess.h>

static volatile unsigned long *pecfg0 = NULL;
static volatile unsigned long *pecfg1 = NULL;
static volatile unsigned long *pedat  = NULL;

#define PE_CFG0 0x01C20890
#define PE_CFG1 0x01C20894
#define PE_DAT  0x01c208A0
#define DRV_NAME "led_driver"

struct led_control_driver
{
	unsigned int major;
	struct class *class;
	struct device *device;
	unsigned int rval;
};

static struct led_control_driver *led_driver;

static int led_control_open(struct inode *inode, struct file *file)
{
	
	/* pin for led init */
	/* clears [16:31] bits without effecting the [0:15] bits in the pecfg0 reg */
	*pecfg0 &= ~((0xf<<16)|(0xf<<20)|(0xf<<24)|(0xf<<28));
	/* configure the pe4 - pe7 port pins as gpio pins */
	*pecfg0 |=  (0x1<<16)|(0x1<<20)|(0x1<<24)|(0x1<<28);
	

	/* clears [0:15] bits without effecting the [16:31] bits in the pecfg1 reg */
	*pecfg1 &= ~((0xf<<0)|(0xf<<4)|(0xf<<8)|(0xf<<12));
	/* configure the pe8 - pe11 port pins as gpio pins */
	*pecfg1 |=  (0x1<<0)|(0x1<<4)|(0x1<<8)|(0x1<<12);

	return 0;
}

static ssize_t led_control_write(struct file * file, const char __user * buf, size_t size, loff_t * ppos)
{

    if(copy_from_user(&led_driver->rval, buf, size))
    {
		printk(KERN_ERR "failed to copy_from_user!\n");
		return -EINVAL;
    }

	*pedat &= ~((0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<7)|(0x1<<8)|(0x1<<9)|(0x1<<10)|(0x1<<11));
	*pedat |= (led_driver->rval) << 4;

    
    return size;
}

int led_control_release (struct inode *inode, struct file *file)
{
	*pedat &= ~((0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<7)|(0x1<<8)|(0x1<<9)|(0x1<<10)|(0x1<<11));
	return 0;
}

struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_control_open,
	.write = led_control_write,
	.release= led_control_release,
};


static int __init led_control_test_init(void)
{
	int ret;
	printk(KERN_INFO "led_control_init ok!\n");
	
	led_driver = kmalloc(sizeof(struct led_control_driver), GFP_KERNEL);
	if(NULL == led_driver)
	{
		printk(KERN_ERR "faild to malloc memory\n");
		return -ENOMEM;
	}


	led_driver->major = register_chrdev(0,DRV_NAME,&led_fops);
	if( led_driver->major < 0)
	{
		pr_err("failed to register chrdev\n");
		goto register_err;
	}	
	

	led_driver->class = class_create(THIS_MODULE, "user_leds");
	if (IS_ERR(led_driver->class))
	{
		ret = PTR_ERR(led_driver->class);
		goto class_err;
	}
	

	led_driver->device = device_create(led_driver->class, NULL, MKDEV(led_driver->major,0), NULL, "leds");
	if (IS_ERR(led_driver->device))
	{
		ret = PTR_ERR(led_driver->device);
		goto device_err;
	}

	pecfg0 = ioremap(PE_CFG0, 0x10);
	pecfg1 = ioremap(PE_CFG1, 0x10);
	pedat =  ioremap(PE_DAT,  0x10);
	return 0;
	
device_err:
	class_destroy(led_driver->class);
class_err:
	unregister_chrdev(led_driver->major, DRV_NAME);
register_err:
	kfree(led_driver);
	return ret;	
}

static void __exit led_control_test_exit(void)
{
	printk(KERN_INFO "led_control_exit ok!\n");
	
	unregister_chrdev(led_driver->major, DRV_NAME);
	device_destroy(led_driver->class, MKDEV(led_driver->major,0));
	class_destroy(led_driver->class);
	iounmap(pecfg0);
	iounmap(pecfg1);
	iounmap(pedat);
	kfree(led_driver);
}

module_init(led_control_test_init);
module_exit(led_control_test_exit);

MODULE_LICENSE("GPL");
/******************************************************************
 
	PIO base_addr	----->		0x01c20800


*******************************************************************
	PORT NAME	----->	number(n)	      pins
*******************************************************************
	PORT A(PA)	----->	   0		18 i/o port pins
	PORT B(PB)	----->	   1		24 i/o port pins
	PORT C(PC)	----->	   2		25 i/o port pins
	PORT D(PD)	----->	   3		28 i/o port pins
	PORT E(PE)	----->	   4		12 i/o port pins
	PORT F(PF)	----->	   5		06 i/o port pins
	PORT G(PG)	----->	   6		12 i/o port pins
	PORT H(PH)	----->	   7		28 i/o port pins
	PORT I(PI)	----->	   8		22 i/o port pins
	--------------------------------------------------------
	PORT S(PS)	----->	   9		84 i/o port pins
						for DRAM controller
*******************************************************************
(except PS port)
-> These port pins all are multiplexed(multi-functional).
-> By default all ports are enable with GPIO functionality.

======================================================================
	REG_NAME	OFFSET		Description
======================================================================	
	Pn_CFG0		n*0x24+0x00	Port n configure register 0
					(n = 0 to 9)

	Pn_CFG1		n*0x24+0x04	Port n configure register 1
					(n = 0 to 9)

	Pn_CFG2		n*0x24+0x08	Port n configure register 2
					(n = 0 to 9)

	Pn_CFG4		n*0x24+0x0c	Port n configure register 3
					(n = 0 to 9)

	Pn_DAT		n*0x24+0x10	Port n Data register 0
					(n = 0 to 9)

	Pn_DRV0		n*0x24+0x14	Port n Multi-Driver register 0
					(n = 0 to 9)

	Pn_DRV1		n*0x24+0x18	Port n Multi-Driver register 1
					(n = 0 to 9)

	Pn_PUL0		n*0x24+0x1c	Port n pull register 0
					(n = 0 to 9)

	Pn_PUL1		n*0x24+0x20	Port n pull register 1
					(n = 0 to 9)
========================================================================

Example: in our driver, the base_addr of  PE_CFG0, PECFG1, PE_DAT 
		registers are calculated below
	
	- For PE n=4

	- offset for PE_CFG0 = n*0x24 + 0x00 = 4*0x24 + 0x00 = 0x90

	- base_addr of PE_CFG0	= base_addr of PIO + offset
			     	= 0x01c20800 + 0x90	
				= 0x01c20890

	- offset for PE_CFG1 	= n*0x24 + 0x04 = 4*0x24 + 0x04 = 0x94
	- base_addr of PE_CFG1	= base_addr of PIO + offset
			     	= 0x01c20800 + 0x94	
				= 0x01c20894

	- offset for PE_DAT 	= n*0x24 + 0x10 = 4*0x24 + 0x10 = 0xa0
	- base_addr of PE_DAT	= base_addr of PIO + offset
			     	= 0x01c20800 + 0xa0	
				= 0x01c208a0
****************************************************************************
*/
