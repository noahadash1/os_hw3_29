#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 235
// Set the message of the device driver
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

#define DEVICE_RANGE_NAME "char_dev"
#define BUF_LEN 128
#define DEVICE_FILE_NAME "simple_char_dev"
#define SUCCESS 0



typedef struct channel
{
    unsigned int minorNumber;
    char messageString[128];
    struct channel *next;
    int mesLen;

} channel;

typedef struct channelList
{
    channel *first;
} channelList;

#endif
