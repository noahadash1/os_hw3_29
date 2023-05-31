#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 235
// Set the message of the device driver
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define SUCCESS 0



typedef struct channel
{
    unsigned int ID;
    char messageString[128];
    int mesLen;
    struct channel *next;
} channel;

typedef struct channelList
{
    channel *first;
} channelList;

#endif
