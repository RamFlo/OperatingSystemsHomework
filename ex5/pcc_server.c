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
#include <signal.h>
#include <libgen.h>

#define PRINTABLE_CHARS 95
#define CONNECTION_QUEUE_SIZE 100
#define LOCAL_BUFF_SIZE 4096

typedef struct clientProcNode
{
    pthread_t clientProcThread;
    struct clientProcNode *next;
} clientProcNode;

pthread_mutex_t pcc_count_mutex;
clientProcNode *headNode = NULL;
int listenfd;
unsigned int pcc_count[PRINTABLE_CHARS] = {0};

void printErrorAndExit(const char *errStr)
{
    printf("%s: %s\n", errStr, strerror(errno));
    exit(EXIT_FAILURE);
}

void printPrintableCharsCount()
{
    int i = 0;
    for (i = 0; i < PRINTABLE_CHARS; i++)
        printf("char '%c' : %u times\n", (i + 32), pcc_count[i]);
}

int readDataFromClient(int sockfd, char *readIntoPtr, unsigned int readLength)
{
    unsigned int charsRead = 0, ret = readLength;
    while ((charsRead = read(sockfd, readIntoPtr, readLength)) > 0)
    {
        readLength -= charsRead;
        readIntoPtr += charsRead;
    }
    if (charsRead < 0)
        return -1;
    return ret;
}

int sendDataToClient(int sockfd, char *data, unsigned int dataLength)
{
    unsigned int charsSent = 0, totalCharsSent = 0;
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
    int clientConnfd = (int)((size_t)client_Connfd), i = 0;
    uint32_t clientRetVal, serverRetVal;
    unsigned int charsToRead, totalPrintableCharsFromClient = 0, charsToReadIteration;
    char charsBuffer[LOCAL_BUFF_SIZE];

    if ((readDataFromClient(clientConnfd, (char *)&clientRetVal, sizeof(uint32_t))) < 0)
    {
        close(clientConnfd);
        printErrorAndExit("could not read length of message from client");
    }
    charsToRead = ntohl(clientRetVal);

    while (charsToRead > 0)
    {
        charsToReadIteration = charsToRead < LOCAL_BUFF_SIZE ? charsToRead : LOCAL_BUFF_SIZE;
        if ((readDataFromClient(clientConnfd, charsBuffer, charsToReadIteration)) < 0)
        {
            close(clientConnfd);
            printErrorAndExit("could not read message from client");
        }
        for (i = 0; i < charsToReadIteration; i++)
        {
            if (charsBuffer[i] >= 32 && charsBuffer[i] <= 126)
            {
                totalPrintableCharsFromClient++;
                if (pthread_mutex_lock(&pcc_count_mutex) < 0)
                    printErrorAndExit("pthread_mutex_lock error");
                pcc_count[charsBuffer[i] - 32]++;
                if (pthread_mutex_unlock(&pcc_count_mutex) != 0)
                    printErrorAndExit("Could not unlock mutex");
            }
        }
        charsToRead -= charsToReadIteration;
    }

    serverRetVal = htonl(totalPrintableCharsFromClient);
    if ((sendDataToClient(clientConnfd, (char *)&serverRetVal, sizeof(uint32_t))) < 0)
    {
        close(clientConnfd);
        printErrorAndExit("could not send response to client");
    }
    close(clientConnfd);
    pthread_exit(NULL);
}

int createAndAddNewClientProc(int client_Connfd)
{
    clientProcNode *newClientProcNode = (clientProcNode *)malloc(sizeof(clientProcNode));
    if (newClientProcNode == NULL)
        return -1;
    if (pthread_create(&newClientProcNode->clientProcThread, NULL, thread_clientProc, (void *)(intptr_t)client_Connfd) < 0)
    {
        printf("Could not create thread. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    newClientProcNode->next = headNode;
    headNode = newClientProcNode;
    return 1;
}

void freeClientList(clientProcNode *curNode)
{
    if (curNode == NULL)
        return;
    if (pthread_join(curNode->clientProcThread, NULL) < 0)
        printErrorAndExit("pthread_join error");
    freeClientList(curNode->next);
    free(curNode);
}

void my_signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        close(listenfd);
        freeClientList(headNode);
        printPrintableCharsCount();
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    int connfd;
    unsigned short serverPort;
    struct sigaction new_action;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printErrorAndExit("Failed creating listening socket");

    sscanf(argv[1], "%hu", &serverPort);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverPort);

    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_sigaction = my_signal_handler;
    new_action.sa_flags = SA_SIGINFO;
    if (0 != sigaction(SIGINT, &new_action, NULL))
        printErrorAndExit("Signal handle registration failed");

    if (pthread_mutex_init(&pcc_count_mutex, NULL) != 0)
    {
        printf("Could not initiate mutex. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

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
        if (createAndAddNewClientProc(connfd) < 0)
        {
            close(listenfd);
            printErrorAndExit("Failed creating new client proccessor");
        }
    }

    exit(EXIT_SUCCESS);
}