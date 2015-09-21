
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
#include "hw_test.h"



/**
* loop test dram len bytes  starting at start_addr
*/
static int testdram (unsigned int start_addr,unsigned int len,int loop)
{
    uint *pstart ;
    uint *pend ;
    uint *p;
    int i=0;
    
    if(start_addr!=0 && len !=0){
        pstart = (uint *) start_addr;
        pend = (uint *) (start_addr +len);
        printf("start_addr:0x%0x====>len:0x%0x\n",(unsigned int)pstart,len);
    
    }else{
        pstart = (uint *) CONFIG_TEST_ADDR_START;
        pend = (uint *) (CONFIG_TEST_ADDR_START + CONFIG_DEFAULT_TEST_LEN);
        printf("start_addr:0x%0x====>len:0x%0x\n",(unsigned int)pstart,CONFIG_DEFAULT_TEST_LEN);
    }
    
    
    for(i =0; i< loop; ++i){
        puts ("SDRAM test phase 1:\n");
        for (p = pstart; p < pend; p++)
            *p = 0xaaaaaaaa;
    
        for (p = pstart; p < pend; p++){
            if (*p != 0xaaaaaaaa) {
                printf ("SDRAM test fails at: 0x%08x loop:%d\n", (uint) p,i);
                return 1;
            }
        }
    
        puts ("SDRAM test phase 2:\n");
        for (p = pstart; p < pend; p++)
            *p = 0x55555555;
    
        for (p = pstart; p < pend; p++) {
            if (*p != 0x55555555) {
                printf ("SDRAM test fails at: %08x loop:%d\n", (uint) p,i);
                return 1;
            }
        }
    }
    puts ("SDRAM test passed.\n");
    return 0;
}

/**
* dram test command
*/
static int do_ddr_test(int argc, char * const argv[])
{

    char *endp;
    unsigned long start_addr;
    unsigned long len;
    int loop =1;
    
    /*default ddr test */
    if(argc ==0){
        if(testdram(0,0,1) != 0)
            return -1;
    }else if(argc < 4){
        return -1;
    }else{   
    
        start_addr = simple_strtoul(argv[1],&endp,0);
        len = simple_strtoul(argv[2],&endp,0);  
        loop = simple_strtoul(argv[3],&endp,0);
        
        if(start_addr >= CONFIG_TEST_ADDR_START && 
        start_addr <CONFIG_TEST_ADDR_END && 
        (start_addr +len) < CONFIG_TEST_ADDR_END){
            if(testdram(start_addr,len,loop) == 0)
                return 0;
        }
        else{
            printf("ddr test addr from 0x%0x to 0x%0x\n",CONFIG_TEST_ADDR_START,CONFIG_TEST_ADDR_END);
            return -1;
        }

    }     
return 0;
}

/**
* ddr read for debug
*/
static int do_ddrrd_test(int argc, char * const argv[])
{

    char *endp;
    unsigned long start_addr;
    unsigned long len;
    int loop =1;
    
    uint *pstart ;
    uint *pend ;
    uint *p;
    int i=0;
    uint tmp=0;
	
    if(argc < 4)
        return -1;
    
    start_addr = simple_strtoul(argv[1],&endp,0);
    len = simple_strtoul(argv[2],&endp,0);
    loop = simple_strtoul(argv[3],&endp,0);
    
    if(start_addr >= CONFIG_TEST_ADDR_START &&
    start_addr <CONFIG_TEST_ADDR_END &&
    (start_addr +len) < CONFIG_TEST_ADDR_END){
        for(i =0;i < loop; i++){
            pstart = (uint *) start_addr;
            pend = (uint *) (start_addr +len);
            printf("start_addr:0x%0x====>end_addr:0x%0x\n",(unsigned int)pstart,(unsigned int)pend);
        
            for (p = pstart; p < pend; p++) {
                tmp = *p;    
            }
        
        }
    }
    else{
        printf("ddr test addr from 0x%0x to 0x%0x\n",CONFIG_TEST_ADDR_START,CONFIG_TEST_ADDR_END);
        return -1;
    }

    return 0;
}

static int do_ddrwt_test(int argc, char * const argv[])
{
    char *endp;
    unsigned long start_addr;
    unsigned long len;
    int loop =1;
    
    uint *pstart ;
    uint *pend ;
    uint *p;
    int i=0;
    unsigned long contxt= 0xaaaaaaaa;
    
    if(argc < 5)
        return -1;
    
    start_addr = simple_strtoul(argv[1],&endp,0);
    len = simple_strtoul(argv[2],&endp,0);
    
    if(argc >=4)
        contxt =  simple_strtoul(argv[3],&endp,0);
    if(argc >=5)
        loop = simple_strtoul(argv[4],&endp,0);
    
    if(start_addr >= CONFIG_TEST_ADDR_START &&
    start_addr <CONFIG_TEST_ADDR_END &&
    (start_addr +len) < CONFIG_TEST_ADDR_END){
        pstart = (uint *) start_addr;
        pend = (uint *) (start_addr +len);
        printf("start_addr:0x%0x====>end_addr:0x%0x\n",(unsigned int)pstart,(unsigned int)pend);
        for(i =0;i < loop; i++){
        
            for (p = pstart; p < pend; p++){
                *p = contxt;
            }
        
        }
    }
    else{
        printf("ddr test addr from 0x%0x to 0x%0x\n",CONFIG_TEST_ADDR_START,CONFIG_TEST_ADDR_END);
        return -1;
    }
    return 0;
}
static int fatsystem_test(const char *ifname,const char *part_str,const char *filename,unsigned long filesize,int loop)
{
    char cmdstr[BUFLEN];
    char cmdstr2[BUFLEN];
    unsigned long addr = 0x82000000;
    unsigned long bytes=0;
    char tmpstr[64];
    int i=0;
    unsigned int *pend;
    unsigned int *p;
    
    memset(cmdstr,0,BUFLEN);
    
    /* bytes = strlen(test_str);*/
    bytes = filesize & (~3);
    /*fill bytes length 0xaa to 0x82000000*/
    memset(addr,0xaa,bytes);
    strcat(cmdstr,"fatwrite ");
    sprintf(tmpstr,"%s %s  0x%x %s 0x%lx",ifname,part_str,(unsigned int)addr,filename,bytes);
    strcat(cmdstr,tmpstr);
    printf("%s\n",cmdstr);
    
    memset(cmdstr2,0,BUFLEN);
    memset(tmpstr,0,64);
    strcat(cmdstr2,"fatload ");
    sprintf(tmpstr,"%s %s  0x%lx %s 0x%lx 0 ",ifname,part_str,addr,filename,bytes);
    strcat(cmdstr2,tmpstr);
    printf("%s\n",cmdstr2);
    
    for( i=0;i< loop;++i){
        if(0 != run_command(cmdstr,0))
            goto fail;
        
        /*after wirte to file,clear 0x82000000*/
        memset(addr,0,filesize);   
        
        if(0 != run_command(cmdstr2,0))
            goto fail;
        pend =(unsigned int *)(addr + bytes);
        
        for(p=(unsigned int *)addr;p< pend;p++){
            if( *p!= 0xaaaaaaaa)
                goto fail;
        }
    }
	return 0;
	fail:
    return 1;

}

/**
* udisk test command
*/
static int do_udisk_test(int argc, char * const argv[])
{
    int loop =1;
    char * ifname ="usb";
    char  part_str[32];
    char * filename ="udisktest";
    char cmdstr[BUFLEN];
    unsigned long filesize = 0x80000;
    int devno=0;
    block_dev_desc_t *stor_dev = NULL;  

    if(argc !=0 && argc < 4){
        printf("Input parameters are error\n");
        return -1;
    }
    
    
    if( argc == 4)
    {
        filename = argv[1];
        filesize = simple_strtoul(argv[2],NULL,0);
        loop =simple_strtoul(argv[3],NULL,0); 
    }
    
    
    memset(cmdstr,0,BUFLEN);
    
    /*usb storage init*/
    strcat(cmdstr,"usb reset");
    
    if( 0 != run_command(cmdstr,0))
        goto fail;   


    for (devno = 0; devno < USB_MAX_STOR_DEV; devno++)
    { 
        stor_dev = usb_stor_get_dev(devno);
        
        if (stor_dev && stor_dev->part_type == PART_TYPE_DOS) 
        {
            printf("Usb Mass Storage is found!\n");
            break;
        }
    }
    
    if(devno == USB_MAX_STOR_DEV)
    {
        printf("Usb Mass Storage is not found!\n");
        goto fail;
    }
    
    memset(part_str,0,32);
    sprintf(part_str,"%d:auto",devno);
    printf("patr_str=%s\n",part_str);
	
    if(0 != fatsystem_test(ifname,part_str,filename,filesize,loop))
        goto fail;
    printf("\n udisk test success\n");
    return 0;
    
fail:
    printf("\n udisk test fail \n");
    
    return 1;

}


/**
* sd test command 
*/

static int do_sd_test(int argc, char * const argv[])
{
    int loop =1;
    char * ifname ="mmc";
    char * part_str ="1:auto";
    char * filename ="sdtest";
    unsigned long filesize = 0x80000;

    if(argc !=0 && argc < 4)
    {
        printf("Input parameters are error\n");
        return -1;
    }
    
    if( argc >=4){
        filename = argv[3];
        filesize = simple_strtoul(argv[4],NULL,0);
        loop =simple_strtoul(argv[5],NULL,0);
    }    
    
       
    
    if( 0!= fatsystem_test(ifname,part_str,filename,filesize,loop))
        goto fail;
    printf("\n sd test success\n");
    return 0;
fail:
    printf("\n sd test fail \n");
    return 1;
}

/**
*    emmc test
*/

static int do_emmc_test(int argc, char * const argv[])
{
    
    char cmdstr[BUFLEN];    
    int i =0;
    int j=0; 
    char tmpstr[64]={'\0'}; 
    unsigned long addr = 0x82000000;
    
    char *endp;
    unsigned long start_addr;
    unsigned long len;
    int loop =1;
    unsigned long bytes=0;
    unsigned long offset;
    int m=0;
    unsigned long contxt = 0x55555555; 
    
    /*default emmc test */
    if(argc ==0){
        start_addr = CONFIG_EMMC_TEST_START_ADDR;
        len = CONFIG_EMMC_TEST_LEN;
    }else if(argc < 5){
        printf("Input parameters are error\n");
        return -1;
    }else{
        start_addr = simple_strtoul(argv[1],&endp,0);
        len = simple_strtoul(argv[2],&endp,0);
		 contxt =  simple_strtoul(argv[3],&endp,0);
		 loop = simple_strtoul(argv[4],&endp,0);
    }

    printf("test loop:%d   context:%lx\n",loop,contxt);
    printf("start_addr:0x%lx  end_addr:0x%lx\n",start_addr,(start_addr+len));
    
        
    
    /*valid start_addr and len*/
    if(start_addr >= CONFIG_EMMC_TEST_START_ADDR && 
    start_addr <CONFIG_EMMC_TEST_END_ADDR && 
    (start_addr +len) < CONFIG_EMMC_TEST_END_ADDR){
    
        /*do emmc test*/
        memset(cmdstr,0,sizeof(cmdstr));        
		
        for(i = 0;i <loop; ++i){
            /*emmc erase*/       
            memset(cmdstr,0,sizeof(cmdstr)); 
            strcat(cmdstr,"mmc erase ");
            sprintf(tmpstr,"0x%lx ", start_addr);
            strcat(cmdstr,tmpstr);
            memset(tmpstr,0,sizeof(tmpstr));
            sprintf(tmpstr,"0x%lx ",len);
            strcat(cmdstr,tmpstr);                      
            if(0 != run_command(cmdstr,0))
                goto fail;
            
            /*fill contxt to addr (0x82000000)*/
            
            for( j=0;j< 512*1024;j+=4){
                
                *((unsigned long *)(addr+j)) = contxt;
            }
            
            bytes= len;
            while(bytes != 0){
                memset(cmdstr,0,sizeof(cmdstr));
                memset(tmpstr,0,sizeof(tmpstr)); 
                strcat(cmdstr,"mmc write 0x82000000 ");
                offset = len -bytes;                   
                sprintf(tmpstr,"0x%lx ",(start_addr + offset));
                
                strcat(cmdstr,tmpstr);
                if(bytes >= (512*1024)){ 
                    strcat(cmdstr," 0x80000");
                    bytes -= (512*1024);
                }else{
                    memset(tmpstr,0,sizeof(tmpstr));
                    sprintf(tmpstr," 0x%lx ",bytes);
                    strcat(cmdstr,tmpstr);
                    bytes =0; 
                }
                
                if(0 != run_command(cmdstr,0))
                    goto fail;             
            }   
            
            memset((unsigned char*)addr,0x0,512*1024);
            
            /*read len bytes data form emmc*/
            
            bytes= len;
            while(bytes != 0){
                memset(cmdstr,0,sizeof(cmdstr));
                memset(tmpstr,0,sizeof(tmpstr)); 
                strcat(cmdstr,"mmc read 0x82000000 ");
                offset = len -bytes;                   
                sprintf(tmpstr," 0x%lx ",(start_addr + offset));
                
                strcat(cmdstr,tmpstr);
                if(bytes >= (512*1024)){ 
                    strcat(cmdstr," 0x80000");
                    bytes -= (512*1024);
                    m = (512*1024);
                }else{
                    memset(tmpstr,0,sizeof(tmpstr));
                    sprintf(tmpstr," 0x%lx ",bytes);
                    strcat(cmdstr,tmpstr);
                    m = bytes;
                    bytes =0; 
                }
                
                if(0 != run_command(cmdstr,0))
                    goto fail;
                /*compare the string*/  
                for(j=0; j<m; j+=4){
                    if(contxt != *(unsigned long*)(addr+j)){
                        printf("contxt 0x%lx *addr=0x%lx\n",contxt,*(unsigned long*)(addr+j));  
                        goto fail;
                    }
                }
            
            }   
            
        }
        printf("EMMC test sucess!\n");
        goto ret;
    }
    else{
        printf("EMMC test addr from 0x%0x to 0x%0x\n",CONFIG_EMMC_TEST_START_ADDR,CONFIG_EMMC_TEST_END_ADDR);
        return -1;
    }
    fail:
        printf(" EMMC test fail\n");
    
    ret:	      
        return 0;
}

/**
*   do fpga spi test
*/

static int do_fpga_spi_test(int argc, char * const argv[])
{
    printf("fpga spi test begining\n");
    printf("fpga spi test end\n");
    
    return 0;
}

/**
*   do fpga mmc test
*/

static int do_fpga_mmc_test(int argc, char * const argv[])
{
    printf("fpga mmc test begining\n");
    printf("fpga mmc test end\n");
    
    return 0;
}


/**
*
*/

static int do_lcd_test(int argc, char * const argv[])	
{

    char cmdstr[BUFLEN];
    int loop =1;
    int i=0;

    if(argc != 0 && argc <2){
        printf("Input parameters are error\n");
        return -1;
    }
    if(argc >=2)
        loop = simple_strtoul(argv[1],NULL,0);
    
    memset(cmdstr,0,sizeof(cmdstr));
    sprintf(cmdstr,"lcd ini");
    
    if( 0 != run_command(cmdstr,0))
        goto fail;
    
    memset(cmdstr,0,sizeof(cmdstr));
    sprintf(cmdstr,"lcd bl on");
    
    if( 0 != run_command(cmdstr,0))
        goto fail;
    
    memset(cmdstr,0,sizeof(cmdstr));
    sprintf(cmdstr,"lcd test");
    
    for(i =0; i< loop; ++i){
        if( 0 != run_command(cmdstr,0))
        goto fail;
    }
    printf("lcd test sucess \n");
    return 0;
fail:
    printf("lcd test fail\n");
    return 1;
}


/*register hware table*/

hw_tbl_t hw_table[] =  {
    {
    .name         = "ddr",
    .hwtest      =do_ddr_test,
    },
    
    #if 1
    {
    .name           = "sd",
    .hwtest         = do_sd_test,
    },
    #endif
    #ifdef CONFIG_USB_STORAGE	
    {
    .name            ="udisk",
    .hwtest         = do_udisk_test,
    },
    #endif
    {
    .name          = "emmc",
    .hwtest      = do_emmc_test,
    },
    {
    .name          = "lcd",
    .hwtest      = do_lcd_test,
    },
};


/*fast scan hardware test*/
static int do_all_hardware_test(void)
{

    int i =0;
    int ret =0;
    
    
    for( i =0; i <  ARRAY_SIZE(hw_table);++i){ 
        ret = hw_table[i].hwtest(0,NULL);
        
        if( ret ==0){
            printf( "%s test is sucess!\n",hw_table[i].name);
        }
        else{
            printf( "%s test is failed!\n",hw_table[i].name); 
        }
    
    }
    return ret;

}



static int do_hw_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    const char *cmd;
    int ret;
    
    /* need at least two arguments */
    if (argc < 2)
        goto usage;
    
    cmd = argv[1];
    --argc;
    ++argv;
    
    if (strcmp(cmd, "all") == 0) {
        ret = do_all_hardware_test();
        goto done;
    }
    
    if (strcmp(cmd, "sd") == 0 )
        ret = do_sd_test(argc,argv);
    else if (strcmp(cmd, "ddr") == 0)
        ret = do_ddr_test(argc,argv);
    else if (strcmp(cmd, "udisk") == 0)
        ret = do_udisk_test(argc,argv);
    else if (strcmp(cmd,"fpgaspi") == 0)
        ret = do_fpga_spi_test(argc,argv);
    else if (strcmp(cmd,"fpgammc") == 0)
        ret = do_fpga_mmc_test(argc,argv);
    else if (strcmp(cmd, "lcd") == 0)
        ret = do_lcd_test(argc,argv);
    else if (strcmp(cmd,"emmc") == 0)
        ret = do_emmc_test(argc,argv);
    else if(strcmp(cmd,"ddrrd") == 0)
        ret = do_ddrrd_test(argc,argv);
    else if(strcmp(cmd,"ddrwt") == 0)
        ret = do_ddrwt_test(argc,argv);
    else
        ret = -1;

done:
    if (ret != -1)
        return ret;

usage:
    return CMD_RET_USAGE;
} 


U_BOOT_CMD(
hwtest,	16,	0,	do_hw_test,
"hardware test sub-system",
" all                 - fast scan all hardware\n"
"hwtest ddr  <addr> <len> <nr> 	- 'nr' loops test `len' bytes starting at`addr' \n"
"            'nr' gives the stress test loops. If 'nr' is omitted, 1 is used.\n"
"           DDR test address from 0x80000000 to 0x86e00000\n"
"hwtest ddrrd  <addr> <len> <nr>  -'nr' loops read `len' bytes starting at`addr' \n"
"hwtest ddrwt  <addr> <len> <context> <nr>  - 'nr' loops write `len' bytes starting at`addr' \n"
"             'context'gives the write context.\n "
"hwtest udisk <filename> <filesize> <nr>	- 'nr' loops test for usb storage\n"
"            example: 'hwtest udisk  udisktest 0x8000 1'\n"
"hwtest sd  <filename> <filesize> <nr>	- 'nr' loops test for sd storage\n"
"            example: 'hwtest sd sdtest 0x8000 1'\n"
"hwtest emmc  <addr> <len> <context> <nr>	- 'nr' loops test `len' bytes starting at`addr'\n"
"            EMMC test addr from 0x80000000 to 0xc0000000 \n"
"hwtest lcd  <nr>        - 'nr' loops test lcd display\n"
);
