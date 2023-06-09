#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message_slot.h"

int main(int argc, char const *argv[]) {
    int id;
    if (argc != 3){
        perror("wrong anount of args");
        exit(1);
    }
    int file = open(argv[1], O_RDONLY);
    if (file == -1){
        perror("opening file failed");
        exit(1);
    }
    id = atoi(argv[2]);
    if (ioctl(file, MSG_SLOT_CHANNEL, id) < 0){
        perror("setting channel id failed");
        exit(1);
    }
    char buffer[BUF_LEN];
    int msLen = read(file, buffer, BUF_LEN);
    if (msLen < 0){
        perror("reading message failed");
        exit(1);  
    }
    close(file);
    if (write(1, buffer, msLen) != msLen) {
        perror("writting the massage to stdout faild");
        exit(1);
    }
    exit(0);
}
