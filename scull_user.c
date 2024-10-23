#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "scull_driver/scull_ioctl.h"

#define DEVICE "/dev/scull"
#define BUFFER_SIZE 1024

int main() {
    int fd;
    ssize_t ret;
    char write_buf[] = "Hello from user space!";
    char read_buf[BUFFER_SIZE];

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        printf("Failed to open device for writing\n");
        return EXIT_FAILURE;
    }
    printf("Success to open device for writing\n");
    
    ret = ioctl(fd, SCULL_IO_RESET);
    
    if(ret < -1)
    {
    	printf("Error in SCULL_IO_RESET\n");
    }
    else
    {
    	printf("SCULL_IO_RESET scuccesful\n");
    }
    
    int val;
    
    ret = ioctl(fd, SCULL_IO_GET_QSET, val);
    
    if(ret != -1)
    	printf("scull qset=%d\n", val);
    else
    	printf("error in SCULL_IO_GET_QSET\n");
    	
    ret = ioctl(fd, SCULL_IO_GET_QUANTUM, val);
    
    if(ret != -1)
    	printf("scull quantum=%d\n", val);
    else
    	printf("error in SCULL_IO_GET_QUANTUM\n");
    
    
    val = 10;
    ret = ioctl(fd, SCULL_IO_SET_QUANTUM, &val);
    
    if(ret < -1)
    {
    	printf("Error in SCULL_IO_SET_QUANTUM\n");
    }
    else
    {
    	printf("SCULL_IO_SET_QUANTUM scuccesful\n");
    }
    
    ret = ioctl(fd, SCULL_IO_SET_QSET, &val);
    
    if(ret < -1)
    {
    	printf("Error in SCULL_IO_SET_QSET\n");
    }
    else
    {
    	printf("SCULL_IO_SET_QSET scuccesful\n");
    }
    
    
    
    val = 5;
    
    ret = ioctl(fd, SCULL_IO_TELL_QUANTUM, val);
    
    if(ret < -1)
    {
    	printf("Error in SCULL_IO_TELL_QUANTUM\n");
    }
    else
    {
    	printf("SCULL_IO_TELL_QUANTUM scuccesful\n");
    }
    
    ret = ioctl(fd, SCULL_IO_TELL_QSET, val);
    
    if(ret < -1)
    {
    	printf("Error in SCULL_IO_TELL_QSET\n");
    }
    else
    {
    	printf("SCULL_IO_TELL_QSET scuccesful\n");
    }
    
    
    
    return 0;
}
