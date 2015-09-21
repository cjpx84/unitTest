/************************************************************************
* $Header:
* DESCRIPTION:
*     This file is the public head file for the spi_mcu devices driver on Prism board. 
*     The public data struct and cmd macro define are used for calling the 
*     spi_mcu driver's api to configure these spi devices
*
*     Author: wangbinbin     Date: 07/23/2014
*/

#ifndef SPIMCU_PUBLIC_H_INCLUDED
#define SPIMCU_PUBLIC_H_INCLUDED    

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

typedef struct {
	unsigned char     cmd;		
	unsigned long     address;					
	unsigned int	   val;				
} MCUDATATYPE,*PMCUDATATYPE;

#define SPIMCU_DEVS_NAME  "/dev/spi_mcu"

/*
 * Ioctl definitions
 */
/* Use 'M' as magic number */
#define SPIMCU_IOC_MAGIC                       'M'
#define SPIMCU_IOCX_READ 			_IOWR(SPIMCU_IOC_MAGIC, 0 , int)
#define SPIMCU_IOCX_WRITE  		        _IOWR(SPIMCU_IOC_MAGIC, 1 , int)
#define SPIMCU_IOC_GET_INTERRUPT	_IO(SPIMCU_IOC_MAGIC, 2 )

#define SPIMCU_IOC_MAXNR 6
#define SPIMCU_INVALID_VALUE 0x1234abcd

/*brief Commands codes.*/
enum cmd_tags{
    CMD_DEFAULT = 0x0,
    CMD_RTC     =  0x1,
    CMD_CHARGE  = 0x2,
    CMD_BATTERY  = 0x3,
    CMD_BUZZER  = 0x4,
    CMD_POWER  = 0x5,
    CMD_INTERRUPT =0x6,
    CMD_CONFIG_FILED = 0x7,
    CMD_SYSTEM_RESET = 0x8,
    CMD_CHARGER_ENABLE = 0x9,
    CMD_MCU_VERSION  = 0x10,
    CMD_MCU_VERNUM  = 0x11,
    CMD_RED_LED        = 0x12,
    CMD_GREEN_LED      = 0x13,
};
#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif //SPIMCU_PUBLIC_H_INCLUDED    








