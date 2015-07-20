#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>


int main()
{
	unsigned int val = 0b1111;
	int fd,i;
	unsigned long long delay = 500*1000;
	fd = open("/dev/leds",O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}
	
	write(fd, &val, 4);
	usleep(delay);

	while(1)
	{
		for(i=0; i<4; i++)
		{
			val = val << 1;
			write(fd, &val, 4);
			usleep(delay);
		}
		for(i=0; i<4;i++)
		{
			val = val >> 1;
			write(fd, &val, 4);
			usleep(delay);
		}
		
	}
	close(fd);
	return 0;
}

