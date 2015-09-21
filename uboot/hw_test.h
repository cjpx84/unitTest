/*
* the header file for hardware test commands
*
* (c) Copyright Tektronix Inc., All Rights Reserved
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#ifndef _HW_TEST_H_
#define _HW_TEST_H_

#ifdef __cpluseplus
extern "C"{
#endif


/*hardware tables*/
struct hw_tbl_s {
    char            *name;          /* hardware Name*/
    /* Implementation function      */
    int             (*hwtest)(int argc, char * const argv[]);
};

typedef struct hw_tbl_s hw_tbl_t;

/*ddr test*/

#define CONFIG_TEST_ADDR_START CONFIG_SYS_SDRAM_BASE
#define CONFIG_DEFAULT_TEST_LEN   (0x100000)
#define  CONFIG_TEST_ADDR_END (0x86e00000)


/*udisk test*/
#define BUFLEN 128

/*emmc test*/
#define CONFIG_EMMC_TEST_START_ADDR (0x80000000)
#define CONFIG_EMMC_TEST_LEN (640*1024)
#define CONFIG_EMMC_TEST_END_ADDR (0xc0000000)


#ifdef __cpluseplus
}
#endif
#endif
