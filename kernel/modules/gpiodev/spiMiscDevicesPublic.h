/************************************************************************
* $Header:
* DESCRIPTION:
*     This file is the public head file for the misc devices driver all contolled by the spi bus on
*     Cornerstone board. The public data struct and cmd macro define are used for calling the 
*     spiMiscDevs driver's api to configure these spi devices
*
*     Author: wangbinbin     Date: 03/19/2014
*/

#ifndef SPIDEVS_PUBLIC_H_INCLUDED
#define SPIDEVS_PUBLIC_H_INCLUDED  

#ifndef _GSOP_BUILD_
#include "project.h"
#endif  

#define LGNGTH_MAX     256

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

typedef struct _spidevsdata_
{   
    unsigned int devNo;
    unsigned int dataLength;
    union
    {
         unsigned char regData[LGNGTH_MAX];
         unsigned char* pFpgaImg;
    }devData;
}SPIDEVSDATATYPE,*PSPIDEVSDATATYPE;

/* 
*  bit[3]: CTRL_CPLD_RLDRIVER4_IN
*  bit[2]: CTRL_CPLD_RLDRIVER3_IN
*  bit[1]: CTRL_CPLD_RLDRIVER2_IN
*  bit[0]: CTRL_CPLD_RLDRIVER1_IN
*/
#define RELAY_NO_INTERLEAVE     (0x5)
#define RELAY_1_3_INTERLEAVE    (0xf)
#define RELAY_1_4_INTERLEAVE    (0x3)
#define RELAY_2_3_INTERLEAVE    (0xc)
#define RELAY_2_4_INTERLEAVE    (0x0)

#define SPI_MISC_DEVS_NAME  "/dev/spiMiscDevs"
/* ioctls  cmd*/
#define SPIMISCDEVS_CMD_MAGIC       (0x44420000)
#define _SPIMISCDEVS_CMD(magic,num) ((magic) + (num))

enum
{
    PLL_CLK_CONFIG = _SPIMISCDEVS_CMD(SPIMISCDEVS_CMD_MAGIC, 1), 
    DAC_VOLT_CONFIG,    
    ADC_SAMPLE_CONFIG,
    FPGA_IMG_CONFIG,
    RELAY_INTERLEAVE_CONFIG
};

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif //SPIDEVS_PUBLIC_H_INCLUDED  

