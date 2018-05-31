#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "message_slot.h"

int main(int argc, char* argv[])
{
    int fileDesc,charsRead=0;
    char msgBuff[MSG_SIZE+1];
    if (argc<3)
    {
        printf("Error: not enough arguments\n");
		exit(EXIT_FAILURE);
    }
    fileDesc = open(argv[1], O_RDONLY);
    if (fileDesc < 0)
	{
		printf("Could not open file: filename = %s. %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
    if ((ioctl(fileDesc,MSG_SLOT_CHANNEL,atoi(argv[2])))<0)
    {
        printf("Could not change device channel: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    if ((charsRead=read(fileDesc,msgBuff,MSG_SIZE))<0)
    {
        printf("Could not read message: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    msgBuff[charsRead]='\0';
    close(fileDesc);
    printf("message read: %s\n",msgBuff);
    printf("%d bytes read from %s\n",charsRead,argv[1]);
    exit(EXIT_SUCCESS);
}