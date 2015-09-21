/************************************************************************
* $Header:
* DESCRIPTION:
*     This file is the public head file for the spi_fpga devices driver on Prism board. 
*     The public data struct and cmd macro define are used for calling the 
*     spi_fpga driver's api to configure these spi devices
*
*     Author: wangbinbin     Date: 07/23/2014
*/

#ifndef SPIFPGA_PUBLIC_H_INCLUDED
#define SPIFPGA_PUBLIC_H_INCLUDED

#ifndef _GSOP_BUILD_
#include "project.h"
#endif

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

typedef struct {
	unsigned short     addr;		/*address of register*/
	unsigned short     val;					
	unsigned int	   dev;		/*dev: 00b-fpga; 01b-ADC; 10b-1wire*/		
} SPIDATATYPE,*PSPIDATATYPE;

#define SPIFPGA_DEVS_NAME  "/dev/spi_fpga"

#define FPGA_DEV     0
#define ADC_DEV       1
#define ICON_DEV0   2
#define ICON_DEV1   3

/*
 * Ioctl definitions
 */
/* Use 'S' as magic number */
#define SPIFPGA_IOC_MAGIC                      'S'
#define SPIFPGA_IOCX_READ 			_IOWR(SPIFPGA_IOC_MAGIC, 0 , int)
#define SPIFPGA_IOCX_WRITE  		        _IOWR(SPIFPGA_IOC_MAGIC, 1 , int)
#define SPIFPGA_IOC_GET_INTERRUPT	_IO(SPIFPGA_IOC_MAGIC, 2 )   //reserved
#define SPIFPGA_IOC_LOAD_DONE    _IOR(SPIFPGA_IOC_MAGIC, 3,int )
#define SPIFPGA_IOC_RELOAD       _IOWR(SPIFPGA_IOC_MAGIC, 4 , int)

#define SPIFPGA_IOC_MAXNR 6

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif //SPIFPGA_PUBLIC_H_INCLUDED    


