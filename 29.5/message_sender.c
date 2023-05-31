#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message_slot.h"

int main(int argc, char const *argv[]) {
    printf("start of sender");
    int file, msLen;
    if (argc != 4){
        perror("wrong anount of args");
        exit(1);
    }
    file = open(argv[1], O_WRONLY);
    if (file == -1){
        perror("opening file failed");
        exit(1);
    }
    printf("mid of sender");
    if (ioctl(file, MSG_SLOT_CHANNEL, argv[2]) < 0){
        perror("setting channel id failed");
        exit(1);
    }
    printf("almost there");
    msLen = strlen(argv[3]);
    if(write(file, argv[3], msLen) != msLen){
        perror("writting message failed");
        exit(1);  
   }
    printf("end of sender");
    close(file);
    exit(0);
}
