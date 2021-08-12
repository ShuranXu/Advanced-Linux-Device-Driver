#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/ioctl.h>


#define MAGIC 'z'
#define GETINV  _IOR(MAGIC, 1, int *)
#define SETLED  _IOW(MAGIC, 2, int *)
#define SETINV  _IOW(MAGIC, 3, int *)

#define LED_SCROLL      0x01
#define LED_NUML        0x02
#define LED_CAPSL       0x04
#define ALL_LEDS_ON     0x07

#define BLINKPLUS "/dev/blinkplus"

int main(int argc, char* argv[])
{
	int fd;
	int i=0;
	unsigned int led;
	unsigned long intv=1;
	int result;
	int cmd;
	unsigned int ledintv;
	
	fd = open(BLINKPLUS,O_RDWR);
	if(fd == -1)
	{
		printf("Error in open\n");
		return -1;
	}
	/* No error checking for failed ioctl is done. Demo only */
	while (1){
        printf("\nHit return..");
        getchar();
		printf("\n Setting SCROLL LOCK\n");
		led= LED_SCROLL;
		result = ioctl(fd, SETLED,&led );
	}
    close(fd);
}