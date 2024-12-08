#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "scull_driver/scull_ioctl.h"

#define DEVICE "/dev/scull_pipe"
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
    
    if(read(fd, read_buf, BUFFER_SIZE) < 0)
    {
    	perror("Failed to read from device\n");
    	close(fd);
    }
    
    close(fd);
    
    return 0;
}
