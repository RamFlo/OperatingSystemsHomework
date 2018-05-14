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
    int fileDesc,charsWritten=0;
    if (argc<4)
    {
        printf("Error: not enough arguments\n");
		exit(EXIT_FAILURE);
    }
    fileDesc = open(argv[1], O_WRONLY);
    if (fileDesc < 0)
	{
		printf("Could not open file: filename = %s. %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
    if (ioctl(fileDesc,MSG_SLOT_CHANNEL,atoi(argv[2])<0))
    {
        printf("Could not change device channel: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    if ((charsWritten=write(fileDesc,argv[3],strlen(argv[3])))<0)
    {
        printf("Could not write to file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
    }
    close(fileDesc);
    printf("%d bytes written to %s\n",charsWritten,argv[1]);
    exit(EXIT_SUCCESS);
}