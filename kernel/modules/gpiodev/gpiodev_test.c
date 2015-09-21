/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>


#include<spiFpgaPublic.h>


static const char *device = "/dev/gpiodev";




int main(int argc, char *argv[])
{
    int ret = 0;
    int fd;
    int done;
    int value;
           
     if(argc < 2){
        printf("arg number error\n");
        return;
    }
    
    fd = open(device, O_RDWR);
    printf("fd=%d\n",fd);
    if(fd == -1){
        printf("can not open file\n");
        return -1;
    } 
    
       if(strcmp(argv[1],"r")== 0){         
            ret = ioctl(fd,SPIFPGA_IOC_LOAD_DONE,&done);
            if (ret == -1){
            printf("can't read interrupt status from mcu\n");
            }else{
                printf(" done status value=0x%x\n",done);
            }
       }
      
       if(strcmp(argv[1],"w")== 0)
       	{       
            ret = ioctl(fd,SPIFPGA_IOC_RELOAD,NULL);
            if (ret == -1){
            printf("write error\n");
            }else{
                printf(" write ok\n");
            }
      }
        
    
    
    close(fd);
    return ret;


}
