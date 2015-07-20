/*
* I2C testing utility
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "eeprom.h"

#define CHIP_ADDR	0x50		
#define I2C_DEV		"/dev/i2c-1"

#define usage_if(a) do { do_usage_if( a , __LINE__); } while(0);
void do_usage_if(int b, int line)
{
	const static char *usage = 
		"I2C-24C04 Read/Write Program Test!\n"
		"Please use the option [-r] or [-w] for test!";
	if(!b)
		return;
	fprintf(stderr, "%s, %d\n", usage, line);
	exit(1);
}

static int read_from_eeprom(struct eeprom *e, int addr, int size)
{
	int ch, i;
printf("-----------------------------------------------------------");
printf("\n addr|   0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F\n");
printf("-----------------------------------------------------------");
	for(i = 0; i < size; ++i, ++addr)
	{
	usleep(1000);
		if( (ch = eeprom_read_byte(e, addr)) < 0)
		{
			printf("read error !");
		}
		
		if ( (i % 16) == 0 ) 
			printf("\n %.4x|  ", addr);
		else if ( (i % 8) == 0 ) 
			printf("  ");
//		printf("%c ", ch);
		printf("%.2x ", ch);
		fflush(stdout);
	}
	printf("\n\n");
printf("-----------------------------------------------------------");
	printf("\n\n");
	
	return 0;
}

static int write_to_eeprom(struct eeprom *e, int addr)
{
	int i, ret;
	for(i=0, addr=0; i<256; i++, addr++)
	{
		if ( (i % 16) == 0 ) 
			printf("\n %.4x|  ", addr);
		else if ( (i % 8) == 0 ) 
			printf("  ");
			
		printf("%.2x ", i);

//		printf("%c ", arr[i%16]);
		fflush(stdout);
//		eeprom_write_byte(e, addr, arr[i%16]);
		eeprom_write_byte(e, addr, i);
//		if ((ret = eeprom_write_byte(e, addr, i)) < 0)
//			printf("write error");
	}
	printf("\n\n");
	
	return 0;
}

int main(int argc, char** argv)
{
	struct eeprom e;
	int opt = 0;
	int ret = 0;

	usage_if(argc != 2 || argv[1][0] != '-' || argv[1][2] != '\0');
	opt = argv[1][1];

	printf("Open /dev/i2c-1 with 8bit mode\n");
	if ((ret = eeprom_open(I2C_DEV, CHIP_ADDR, &e)) < 0)
	{
		printf("unable to open eeprom device file\n");
		exit(0);
	}
	
	switch(opt)
	{
		case 'r':
			printf(" Reading 256 bytes from 0x0\n");
			read_from_eeprom(&e, 0, 256);
			break;
		case 'w':
			printf(" Writing 0x00-0xff into 24C04 \n");
			write_to_eeprom(&e, 0);
			break;
		default:
			usage_if(1);
			exit(1);
		}
		eeprom_close(&e);

	return 0;
} 

