#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <omap/spimisc/spiFpgaPublic.h>



//#define GPIODEV_MAJOR 199         
//#define GPIODEV_MINOR 0         
#define DEVICE_NAME "gpiodev" 
#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

static int major;
static struct cdev *gpiodev;



static struct class *gpiodev_class;

static struct gpio fpga_gpios[]={
    {GPIO_TO_PIN(1,16),GPIOF_DIR_IN, "FPGA_DONE"},
    {GPIO_TO_PIN(1,21),GPIOF_OUT_INIT_HIGH, "FPGA_PROG"},
};

static void fpga_reload(void)
{ 
    gpio_set_value_cansleep(GPIO_TO_PIN(1,21), 0);
    msleep(3);
    gpio_set_value_cansleep(GPIO_TO_PIN(1,21),1);
}

static int fpga_is_load_done(void)
{ 
    return gpio_get_value_cansleep(GPIO_TO_PIN(1,16));
}

static int gpiodev_open(struct inode *inode,struct file *file)
{
    printk("gpiodev open success!\n");
    return 0;
}

static int gpiodev_release(struct inode *inode, struct file *filp)
{
  printk (" gpiodev released\n");
  return 0;
}

static ssize_t gpiodev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

static ssize_t gpiodev_write(struct file *filp, const char __user *buf,size_t count, loff_t *f_pos)
{
     return 0;

}

static long gpiodev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch(cmd){
        case SPIFPGA_IOC_LOAD_DONE:
        {
            int done;
            done = fpga_is_load_done();
            printk("kernel :done=%d\n",done);
            if (copy_to_user((void *)arg, &done, sizeof(done)))
            return -EFAULT;
            break;
        }
        case SPIFPGA_IOC_RELOAD:
        {
            fpga_reload();
            break;
        }
        
        default:  
            return -ENOTTY;
        }
	
    return 0;
}
        
static const struct file_operations gpiodev_fops =
{
  .owner = THIS_MODULE,
  .open = gpiodev_open,
  .release = gpiodev_release,
  .read = gpiodev_read,
  .write = gpiodev_write,
  .unlocked_ioctl = gpiodev_ioctl,
};

static int __init gpiodev_init(void) 
{
    int ret=0;
    dev_t devno;
    struct device *dev;
    ret = gpio_request_array(fpga_gpios, ARRAY_SIZE(fpga_gpios));
    if (ret < 0) {
        printk( "GPIO request failed\n");
        goto request_failed; 
    }

    /*step1: alloc and register  char device number*/
    #if defined(GPIODEV_MAJOR)
    /*static alloc char device number*/
    major = GPIODEV_MAJOR;
    devno = MKDEV(major,GPIODEV_MINOR);
    ret =register_chrdev_region(devno,1,DEVICE_NAME);
    if(ret !=0){
        printk("register_chrdev_region failed\n");
        goto request_failed;
    }
    #else
    /*dynamic alloc char device number*/
    ret=alloc_chrdev_region(&devno,0,1,DEVICE_NAME); 
    if(ret < 0){
        printk("alloc_chrdev_region failed\n");
        goto request_failed;
    }
    major = MAJOR(devno);
    printk("devno=%d\n",devno);
    #endif
    /* step2 :alloc char device*/
    gpiodev = cdev_alloc();
    if(gpiodev == NULL){
        printk( "Cannot alloc cdev\n");
        goto alloc_failed;
    }
    
    gpiodev->owner=THIS_MODULE;
    gpiodev->ops = &gpiodev_fops;
   
    /*step3: register a char device*/
    ret=cdev_add(gpiodev,devno,1);
    if(ret!=0)
        goto add_failed;

    /*step4: create a class for device*/ 
    /*it can be create dir /sys/class/gpiodev*/
    gpiodev_class = class_create(THIS_MODULE, DEVICE_NAME);
    if(IS_ERR(gpiodev_class)){
        printk("class create failed\n");
        goto class_failed;
    }
    /*step5: */
    dev =device_create(gpiodev_class, NULL, devno, NULL, DEVICE_NAME);
    if(IS_ERR(dev)){
        printk("class device_create failed\n");
        goto device_failed;
    }
    return ret;
device_failed:
    class_destroy(gpiodev_class);
class_failed:
    cdev_del(gpiodev);
    goto alloc_failed;
add_failed:
    kobject_put(&gpiodev->kobj);
alloc_failed:
    unregister_chrdev_region(devno, 1);
request_failed:
    gpio_free_array(fpga_gpios, ARRAY_SIZE(fpga_gpios)); 
return ret;
}

static void __exit gpiodev_exit(void)
{
    printk("gpiodev_exit\n");
    dev_t devno = MKDEV(major, 0);
    printk("devno=%d\n",devno);
    device_destroy(gpiodev_class, devno);
    class_destroy(gpiodev_class);
    unregister_chrdev_region(devno,1);
    cdev_del(gpiodev);
    gpio_free_array(fpga_gpios, ARRAY_SIZE(fpga_gpios));
}

module_init(gpiodev_init);
module_exit(gpiodev_exit);


MODULE_AUTHOR("jiazheng li>");
MODULE_DESCRIPTION("fpga gpiodeve");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:spidev");


