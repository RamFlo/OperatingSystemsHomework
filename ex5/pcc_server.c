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
#include <stdint.h>
#include <pthread.h>

#define PRINTABLE_CHARS 95
#define CONNECTION_QUEUE_SIZE 100

typedef struct clientProcNode
{
    pthread_t clientProcThread;
    int clientConnfd;
    struct clientProcNode *next;
} clientProcNode;

clientProcNode *headNode = NULL;
int listenfd;

int readDataFromClient(int sockfd, char *readIntoPtr, int readLength)
{
    int charsRead = 0, ret = readLength;
    while ((charsRead = read(sockfd, readIntoPtr, readLength)) > 0)
    {
        readLength -= charsRead;
        readIntoPtr += charsRead;
    }
    if (charsRead < 0)
        return -1;
    return ret;
}

int sendDataToClient(int sockfd, char *data, int dataLength)
{
    int charsSent = 0, totalCharsSent = 0;
    while (dataLength > 0)
    {
        charsSent = write(sockfd, data + totalCharsSent, dataLength);
        if (charsSent == -1)
            return -1;
        dataLength -= charsSent;
        totalCharsSent += charsSent;
    }
    return totalCharsSent;
}

void *thread_clientProc(void *client_Connfd)
{
    int clientConnfd = (int)((size_t)client_Connfd);
    uint32_t clientRetVal;
    unsigned int charsToRead

    if ((readDataFromServer(sockfd, (char *)&clientRetVal, sizeof(uint32_t))) < 0)
    {
        close(sockfd);
        printErrorAndExit("could not read response from server");
    }

    serverRetVal = ntohl(clientRetVal);
}

int createAndAddNewClientProc(int client_Connfd)
{
    clientProcNode *newClientProcNode = (clientProcNode *)malloc(sizeof(clientProcNode));
    if (newClientProcNode == NULL)
        return -1;
    newClientProcNode->clientConnfd = client_Connfd;
    if (pthread_create(&newClientProcNode->clientProcThread, NULL, thread_clientProc, (void *)client_Connfd) < 0)
    {
        printf("Could not create thread. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    headNode = newClientProcNode;
    return 1;
}

void freeClientList(clientProcNode *curNode)
{
    if (curNode == NULL)
        return;
    if (pthread_join(curNode->clientProcThread, NULL) < 0)
        printErrorAndExit("pthread_join error");
    close(curNode->clientConnfd);
    freeCientList(curNode->next);
    free(curNode);
}

unsigned int pcc_count[PRINTABLE_CHARS] = {0};

void printErrorAndExit(const char *errStr)
{
    printf("%s: %s\n", errStr, strerror(errno));
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int connfd;
    unsigned short serverPort;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printErrorAndExit("Failed creating listening socket");

    sscanf(argv[1], "%hu", &serverPort);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverPort);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) == -1)
    {
        printErrorAndExit("Failed binding socket");
        close(listenfd);
    }

    if (listen(listenfd, CONNECTION_QUEUE_SIZE) == -1)
    {
        perror("Failed to start listening to incoming connections");
        close(listenfd);
        return EXIT_FAILURE;
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(struct sockaddr_in);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (connfd == -1)
        {
            close(listenfd);
            printErrorAndExit("Failed accepting client connection");
        }

        int bytes_to_send = strlen(msg);
        while (bytes_to_send > 0)
        {
            int bytes_sent = write(connfd, msg, bytes_to_send);
            if (bytes_sent == -1)
            {
                perror("Failed sending message to client");
                close(connfd);
                close(listenfd);
                return EXIT_FAILURE;
            }
            bytes_to_send -= bytes_sent;
            msg += bytes_sent;
        }

        //close(connfd);
    }

    exit(EXIT_SUCCESS);
}