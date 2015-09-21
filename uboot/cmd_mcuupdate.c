/*
 * hardware test commands
 *
 * (c) Copyright Tektronix Inc., All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#include<common.h>
#include<command.h>

#include<part.h>
#include<fat.h>
#include <usb.h>
#include <asm/errno.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <spi.h>
#include <asm/arch/mux.h>

#include "mcu_update.h"

#define SOC_GPIO_0_REGS                      (0x44E07000)
#define MCU_FRAME_LEN   (9)
#define CMD_RTC (1)
#define WRITE (0)
#define READ (1)

extern void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux);
extern int spi_set_bus_width(struct spi_slave *slave, int wordlen);


static struct module_pin_mux spi1_pin_mux[] = {
	{OFFSET(mcasp0_aclkx), (MODE(3) | RXACTIVE | PULLUDEN)},	/* SPI1_SCLK */
	{OFFSET(mcasp0_fsx), (MODE(3) | RXACTIVE | PULLUDEN | PULLUP_EN)},			/* SPI1_D0 */
	{OFFSET(mcasp0_axr0), (MODE(3) | RXACTIVE | PULLUDEN|PULLUP_EN)},	/* SPI1_D1 */
	{OFFSET(mcasp0_ahclkr), (MODE(3) | RXACTIVE | PULLUDEN|PULLUP_EN)},/* SPI1_CS0 */
	{-1},
};

static struct module_pin_mux mcu_int_mux[]={
	{OFFSET(xdma_event_intr1), (MODE(7) | RXACTIVE | PULLUDEN|PULLUP_EN)},
	{-1},

};




int spi_mcu_read(struct spi_slave *spi, unsigned char cmd, unsigned int addr, unsigned int* value)
{

     unsigned char  dout[MCU_FRAME_LEN]={0};
	 unsigned char din[MCU_FRAME_LEN]={0};
    unsigned int bitlen;
	
    bitlen = MCU_FRAME_LEN*8;
	
	cmd =(cmd << 1) |READ;
    memcpy(&dout[0],&cmd,1);
	memcpy(&dout[1],&addr,4);
    memset(&dout[5],0,4);
	
    if(spi_xfer(spi, bitlen, &dout, &din,SPI_XFER_BEGIN |SPI_XFER_END ) != 0)
    {
        printf("3 Error during SPI transaction\n");
        return -1;
    }

	/*wait mcu data ready*/
   while( (readl(SOC_GPIO_0_REGS+0x138) & (1<<20))); 

   
    memset(&dout,0,sizeof(dout));
  
	memset(&din,0,sizeof(din));
  
    if(spi_xfer(spi, bitlen, dout, din,SPI_XFER_BEGIN | SPI_XFER_END) != 0)
    {
        printf("3 Error during SPI transaction\n");
        return -1;
    }
    
    
    printf("recdata:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",din[0],din[1],
    din[2],din[3],din[4],din[5],din[6],din[7],din[8]);
    *value =(din[8]<<24)|(din[7]<<16)|(din[6] <<8)|din[5];
    printf("val=0x%x\n",*value);

	return 0;
}



int spi_mcu_write(struct spi_slave *spi, unsigned char cmd,unsigned int addr, unsigned int value)
{
    unsigned char  dout[MCU_FRAME_LEN]={0};
	unsigned char din[MCU_FRAME_LEN]={0};
    unsigned int bitlen;
    bitlen = MCU_FRAME_LEN*8;
    
    cmd =(cmd << 1) | WRITE;
    
    memcpy(&dout[0],&cmd,1);
	memcpy(&dout[1],&addr,4);
    memcpy(&dout[5],&value,4);
	printf("dout:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",dout[0],dout[1],
    dout[2],dout[3],dout[4],dout[5],dout[6],dout[7],dout[8]);
    return spi_xfer(spi, bitlen, dout, din,SPI_XFER_BEGIN | SPI_XFER_END); 
    	
}



static int spi_mcu_pin_init(void)
{
    configure_module_pin_mux(spi1_pin_mux);
	configure_module_pin_mux(mcu_int_mux);
    return 0;
}

static struct spi_slave* spi_mcu_init(void)
{
    struct spi_slave*  spi = NULL; 
    spi = spi_setup_slave(1,0,12000000,SPI_MODE_1);
    if(NULL == spi)
    {
        printf("Malloc spi slave failed\n");
        return NULL;
    }
    
    spi_set_bus_width(spi,8);
    spi_claim_bus(spi);
	return spi;
}

static void spi_mcu_free(struct spi_slave* spi)
{
    spi_release_bus(spi);
    spi_free_slave(spi);
}
void spi_test()
{
    
    struct spi_slave*  spi1=NULL;
    unsigned int value=0x12345678;
    unsigned int retval=0;
    printf("spi begin\n");
    
    spi_mcu_pin_init();
    spi1 =spi_mcu_init();
    
    if(spi1 == NULL)
        return;
	spi_mcu_read(spi1,CMD_RTC,0,&retval);
	printf("retval=0x%x\n",retval);

	udelay(1000*1000);


    spi_mcu_write(spi1,CMD_RTC,0,value);

    udelay(1000*1000);
    retval=0;
	spi_mcu_read(spi1,CMD_RTC,0,&retval);
	printf("retval=0x%x\n",retval);
	
    spi_mcu_free(spi1);
    
    printf("spi end\n");
}

static int do_mcu_update(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
	int ret;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "update") == 0) {
		printf("test mcu update\n");
		spi_test();
		ret =0;
		goto done;
	}

	

done:
	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
} 
U_BOOT_CMD(
	mcu,	16,	0,	do_mcu_update,
	"mcu update  sub-system",
	"mcu update <addr> <len>  	- update mcu version form 'addr' and 'len' is the version length \n"
	
);
