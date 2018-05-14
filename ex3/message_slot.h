#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 244
#define DEVICE_RANGE_NAME "message_slot"
#define SLOT_CHANNELS 4
#define MSG_SIZE 128
#define SUCCESS 0

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)


typedef struct minorMsgSlotNode{
    int minorNumber;
    //int closed;
    //int channel;
    int msgSizesArray[SLOT_CHANNELS];
    char messageSlotArray[SLOT_CHANNELS][MSG_SIZE];
    struct minorMsgSlotNode* next;
} minorMsgSlotNode;
#endif