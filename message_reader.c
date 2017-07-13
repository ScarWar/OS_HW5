#include "message_slot.h"

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PRINT_ERROR(s) printf("[Error] - %s, %s\n", s, strerror(errno))

int main(int argc, char ** argv) {
    int fd, channel_index, __n_byte_read;
    char buff[MSG_LEN] = {0};

    /* Check number of arguments */
    if (argc != 2){
        PRINT_ERROR("Invalid number of arguments, expected 1");
        exit(-1);
    }

    /* Parse channel index argument and check for error */
    channel_index = strtol(argv[1], NULL, 10);
    if(errno == ERANGE){
        PRINT_ERROR("Failed to parse integer argument");
        exit(-1);
    }

    /* Check validality of channel_index */
    if (!(0 <= channel_index && channel_index <= 3)){
        printf("Invalid argument - argument range is 0 to 3\n");
        exit(-1);
    }   

    /* Open device file */
    fd = open("/dev/"DEVICE_FILE_NAME, O_RDONLY);
    if (fd < 0) {
        PRINT_ERROR("Failed to open device file");
        exit(-1);
    }

    /* Set message slot channel */
    if (ioctl(fd, IOCTL_SET_CHA, channel_index) < 0 ) {
        PRINT_ERROR("ioctl_set_msg failed");
        exit(-1);
    }

    /* Read from message slot */
    __n_byte_read = read(fd, buff, MSG_LEN);
    if (__n_byte_read < 0){
        PRINT_ERROR("Failed to read from message slot");
        close(fd);
        exit(-1);
    }
    buff[__n_byte_read] = 0;

    printf("%s\n", buff);
    close(fd);
    return 0;
}
