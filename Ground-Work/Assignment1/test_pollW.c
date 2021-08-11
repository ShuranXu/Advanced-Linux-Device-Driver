#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/errno.h>

#define BUFSIZE 23
int main(void)
{
	int ret, wfd;
    const char *wbuf = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // const char *wbuf = "hello";
    wfd = open("/dev/poll_dev", O_WRONLY);
    while(1) {
    
        ret = write(wfd, wbuf, strlen(wbuf));
        if(ret == -1){
            perror("write");
            break;
        }
        printf("\n WROTE %d byte|size of buffer %ld\n", ret, strlen(wbuf)); 
        if(ret < strlen(wbuf)){
            printf("\nNeeds to write addtional %ld bytes\n", (strlen(wbuf) - ret));
            wbuf += ret;
        }
        else{
            printf("\n All data is written\n");
            break;
        }
    }

    close (wfd);    
    exit(EXIT_SUCCESS);
}
