

#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include "linux/spimcu/spiMcuPublic.h"


/*************************************
*transfer format need discuss with HW engineer.
*
* spi_mcu_read16
* 
*
*spi_mcu_write16
*
*
*
**************************************/

static int debug_info = 0;

#define MY_NAME	"SPI_MCU"
#define MSG(fmt, arg...) do { \
if (debug_info) \
	printk("%s-------%s: %s: " fmt , __FILE__, MY_NAME , __func__ , ## arg); \
} while (0)

struct spimcu {

	struct spi_device		*spi;
	struct spi_message	msg;
	u32					inter_cond;	
	struct mutex			mutex;
	struct work_struct		workqueue;
};

static struct spimcu *mcu;
static DECLARE_WAIT_QUEUE_HEAD(mcu_res_wait);

static int spi_tansfer(struct spimcu *mcudev, struct spi_transfer *t, unsigned n_xfers)
{
	struct spi_message m;

    struct spi_transfer	*xfer_tmp = t;        
    unsigned n;   

    spi_message_init(&m);
    for (n = 0; n < n_xfers; n++)
    {
        spi_message_add_tail(xfer_tmp++, &m);
    }

	mutex_lock(&mcudev->mutex);
	if (spi_sync(mcudev->spi, &m) != 0 || m.status != 0) {
		mutex_unlock(&mcudev->mutex);
		printk(KERN_ERR "%s: error\n", __func__);
		return -1;
	}
	mutex_unlock(&mcudev->mutex);

	return 0;
}


/*
*	write 32 bits to SDRAM.
*	val	  :	data need writen.
*	return  :
*			-1  spi transfer error happen.
*			0    spi transfer successful.
*/
int spi_mcu_write32(unsigned short dev, unsigned int val)
{
    int ret;
    unsigned int tx[2];

    struct spi_transfer tr[] = {
        {
            .tx_buf = (const void *)&tx[0],
            .len = 2,
            .bits_per_word = 16,
        }, 
        {
            .tx_buf = (const void *)&tx[1],
            .len = 4,
            .bits_per_word = 32,
        },
    };
	//printk( "write address, 0x%x, 0x%x, data: 0x%x\n", dev, offset, val);

    tx[0] = (dev&0xffff);	
    tx[1] = val;

    ret = spi_tansfer(mcu, &tr[0], 2);
	if (ret == -1)
	{
		printk(KERN_ERR"can't send spi message \n");	
		return -1;
	}
	else
		return 0;

}

/*
*	read 32 bits from SDRAM.
*	val	  :	pointer to data read from SDRAM.
*	return  :
*			-1  spi transfer error happen.
*			0    spi transfer successful.
*/
int spi_mcu_read32(unsigned short dev,  unsigned int * val)
{
    int ret;
    unsigned int tx[2];
    unsigned char rx[4];

    struct spi_transfer tr[] = {
        {
            .tx_buf = (const void *)&tx,
            .len = 2,
            .bits_per_word = 16,
            .delay_usecs = 0,   /*time between two transfer is about 24us, if need more delay, add delay_usecs*/
        }, 
        {
            .tx_buf = (const void *)&tx[1],
            .len = 4,
            .rx_buf = (void *)&rx,
            .bits_per_word = 32,
        },
    };    


    tx[0] = (dev&0xffff) ;	
    tx[1] =  0;


    ret = spi_tansfer(mcu, &tr[0], 2);
	if (ret == -1)
	{
		printk(KERN_ERR"can't send spi message \n");	
		return -1;
	}

    *val = (rx[3]<<24)|(rx[2]<<16)|(rx[1] <<8)|rx[0];
    return 0;
}

/*!
 * spi MCU interface - open function
 *
 * @param inode	     struct inode *
 * @param filp	     struct file *
 *
 * @return	     Return 0 on success or negative error code on error
 */
static int spi_mcu_open(struct inode *inode, struct file *filp)
{
	mcu->spi->mode = SPI_MODE_0;
	mcu->spi->max_speed_hz = 12000000;
	spi_setup(mcu->spi);
	return 0;
}

/*!
 * MCU SPI interface - release function
 *
 * @param inode	     struct inode *
 * @param filp	     struct file *
 *
 * @return	     Return 0 on success or negative error code on error
 */
static int spi_mcu_release(struct inode *inode, struct file *filp)
{
	return 0;
}
/*!
 * MCU SPI interface - ioctl function
 *
 * @param inode        structure inode *
 *
 * @param file        structure file *
 * @param cmd      int			command
 * @param arg	 long		ioctl command arg
 *
 * @return  status    0 success, ENODEV invalid device instance,
 *      ENOBUFS failed to allocate buffer, ERESTARTSYS interrupted by user
 */
static long spi_mcu_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;

	/* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
	if (_IOC_TYPE(cmd) != SPIMCU_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > SPIMCU_IOC_MAXNR) return -ENOTTY;

	
	/*
	 * the type is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. Note that the type is user-oriented, while
	 * verify_area is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (ret)
		return -EFAULT;

	switch(cmd) 
	{
		case SPIMCU_IOCX_READ:	
		{
			SPIDATATYPE  info;

			if(0 == argp)
			{
               printk("SPI_MCU read parameter NULL error!\n");
               ret = -EFAULT;
				break;	
			}				
			//get the param structure
			if (copy_from_user(&info, argp, sizeof(info))) {
				ret = -EFAULT;
				break;
			}
            
			if(info.dev > 0x3)
			{
               printk("SPI_MCU read dev parameter wrong!\n");
               ret = -EFAULT;
				break;	
			}

			info.val = 0;
			ret = spi_mcu_read32(info.dev,  (unsigned int *)&info.val);

			//copy the struct return user space
			if(!ret)
			{	
				if (copy_to_user((void *)arg, &info, sizeof(info)))
					ret = -EFAULT;
			}
			break;
		}
		
		case SPIMCU_IOCX_WRITE:	
		{
			SPIDATATYPE  info;

			if(0 == argp)
			{
               printk("SPI_MCU write parameter NULL error!\n");
               ret = -EFAULT;
				break;	
			}
							
			//get the param structure
			if (copy_from_user(&info, argp, sizeof(info))) {
				ret = -EFAULT;
				break;
			}
            
			if(info.dev > 0x3)
			{
               printk("SPI_MCU write dev  parameter wrong!\n");
               ret = -EFAULT;
				break;	
			}		
            
			ret = spi_mcu_write32(info.dev, info.val);

			//copy the struct return user space
			if(!ret)
			{
				if (copy_to_user((void *)arg, &info, sizeof(info)))
					ret = -EFAULT;
			}
			break;
		}

		case SPIMCU_IOC_GET_INTERRUPT:	
		{
            #if 0  //this is option, up to FPGA and application 
			unsigned int resouce;
			wait_event_interruptible(mcu_res_wait,  (mcu->inter_cond & 0x2) != 0);
			MSG(" Wake up Res:0x%x\n", mcu->inter_cond); 
			mcu->inter_cond = mcu->inter_cond & ~(0x2);
			ret = spi_mcu_read32(0x200000+(UP_RESOURCE<<4), &resouce);
			if(ret == 0 )
				return resouce;
			else 
				ret = -EAGAIN;	
            #endif
			break;
		}
	
		
	default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	
	}

	return ret;
}


static void clear_interrupt (struct work_struct *work)
{
#if 0
	int ret = 0;

	ret = spi_mcu_write32((INTERRUPT<<4) + 0x200000, 0x3);
	if(ret == -1)
	{
		printk(KERN_WARNING"%s: error\n", __func__);
	}
	else
	{
		mcu->inter_cond |= 0x2;
		wake_up_interruptible( &mcu_res_wait );
	}
#endif    
}

static irqreturn_t spi_mcu_interrupt(int irq, void *dev_id)
{
	MSG("spi mcu interrupt\n");
	schedule_work(&mcu->workqueue);

	return IRQ_HANDLED;
}

static const struct file_operations spi_mcu_fops = {
        .owner		= THIS_MODULE,
        .unlocked_ioctl = spi_mcu_ioctl,
        .open = spi_mcu_open,
        .release = spi_mcu_release,
};


static struct miscdevice spi_mcu_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "spi_mcu",
	.fops = &spi_mcu_fops,
};

/*
 * register a misc device for mcu and init irq 
 */
static int __devinit spi_mcu_probe(struct spi_device *spi)
{
	//struct resource *spi_resouce = spi->dev.platform_data;
	int err  = 0;

	if (!spi->irq) {
		dev_dbg(&spi->dev, "no IRQ?\n");
		return -ENODEV;
	}
/*
	if (!spi_resouce) {
		dev_dbg(&spi->dev, "no platform data?\n");
		return -ENODEV;
	}
*/	
	mcu = kzalloc(sizeof(struct spimcu), GFP_KERNEL);
	if (!mcu) {
		err = -ENOMEM;
		goto out_err;
	}

	dev_set_drvdata(&spi->dev, mcu);
	mcu->spi = spi;
	mutex_init(&mcu->mutex);
	INIT_WORK(&mcu->workqueue, clear_interrupt);
       if(spi->irq != -1)
       {
	err = request_irq(spi->irq, spi_mcu_interrupt,  
					IRQF_TRIGGER_FALLING, "spi_mcu", &spi_mcu_miscdev);
	if (err != 0) {
		err = -EINVAL;
		printk(KERN_WARNING"request_irq failed for spi mcu%x\n", spi->irq);
		goto out_free_mem;
	}
       }

	err = misc_register(&spi_mcu_miscdev);
	if (err)
		goto out_free_irq;

	MSG("MXC SPI FPGA: minor:0x%x | MAJOR:0x%x\n", spi_mcu_miscdev.minor, MISC_MAJOR);


	return 0;

out_free_irq:	
	free_irq(spi->irq, &spi_mcu_miscdev);
out_free_mem:
	kfree(mcu);
	dev_set_drvdata(&spi->dev, NULL);	
out_err:
	printk(KERN_WARNING"SPI FPGA probe error!--->\n");
	
	return err;
}

static int  spi_mcu_remove(struct spi_device *spi)
{

	MSG("spi_mcu_remove\n");

	free_irq(spi->irq, &spi_mcu_miscdev);	
	misc_deregister(&spi_mcu_miscdev);

	return 0;
}

static struct spi_driver spi_mcu_driver = {
	.driver = {
		   .name = "spi_mcu",
		   .bus = &spi_bus_type,
		   .owner = THIS_MODULE,
		   },
	.probe = spi_mcu_probe,
	.remove	= __devexit_p(spi_mcu_remove),
};

static int __init spi_mcu_init(void)
{
	MSG("Registering the spi mcu driver debug: 0X%x\n", debug_info);
	return spi_register_driver(&spi_mcu_driver);
}

static void __exit spi_mcu_cleanup(void)
{
	spi_unregister_driver(&spi_mcu_driver);
	MSG("spi mcu removed\n");
}


late_initcall(spi_mcu_init);
module_exit(spi_mcu_cleanup);

module_param(debug_info, int, 0644);
MODULE_PARM_DESC(debug_info, "A integer variable");

MODULE_AUTHOR("Tektronix, Inc");
MODULE_LICENSE("GPL"); 

