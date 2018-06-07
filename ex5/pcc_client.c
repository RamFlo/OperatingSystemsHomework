#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

void printErrorAndExit(const char *errStr)
{
    printf("%s: %s\n", errStr, strerror(errno));
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sockfd, randomCharsFd, charsSent;
    unsigned int numOfCharsLeftToSend,sPort,totalCharsSent = 0;
    char *charBuffer;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));

    sscanf(argv[3],"%u",&numOfCharsLeftToSend);
    sscanf(argv[2],"%u",&sPort);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(sPort);

    if (inet_aton(argv[1], &serv_addr.sin_addr) == 0) //if host is not a valid ip address
    {
        hints.ai_family = AF_INET;       /* Allow IPv4 */
        hints.ai_socktype = SOCK_STREAM; /* TCP socket */
        hints.ai_flags = 0;
        hints.ai_protocol = 0; /* Any protocol */
        if (getaddrinfo(argv[1], NULL, &hints, &result) != 0)
            printErrorAndExit("getaddrinfo() failiure");

        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sockfd == -1)
                continue;
            if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
                break;     /* Success */
            close(sockfd); //close unsuccessfull connection socket
        }
        if (rp == NULL) /* No address succeeded */
            printErrorAndExit("Connection to server failed (based on hostname)");
        freeaddrinfo(result); /* No longer needed */
    }
    else
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            printErrorAndExit("could not create socket");
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        {
            close(sockfd);
            printErrorAndExit("Connection to server failed (based on IP address)");
        }
    }
    if (randomCharsFd = open("/dev/urandom", O_RDONLY) < 0)
        printErrorAndExit("could not open /dev/urandom");

    charBuffer = (char *)malloc(sizeof(char) * numOfCharsLeftToSend);
    if (read(randomCharsFd, charBuffer, numOfCharsLeftToSend) != numOfCharsLeftToSend)
        printErrorAndExit("could not read from /dev/urandom");

    while (numOfCharsLeftToSend > 0)
    {
        charsSent = write(sockfd, charBuffer + totalCharsSent, numOfCharsLeftToSend);
        if (charsSent == -1)
        {
            close(sockfd);
            free(charBuffer);
            printErrorAndExit("could not write message to server");
        }
        numOfCharsLeftToSend -= charsSent;
        totalCharsSent += charsSent;
    }
}