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
	unsigned short     dev;		/*dev: 0-rtc; 1-battary; */
	//unsigned short     val;					
	unsigned int	   val;				
} SPIDATATYPE,*PSPIDATATYPE;

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


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif //SPIMCU_PUBLIC_H_INCLUDED    








