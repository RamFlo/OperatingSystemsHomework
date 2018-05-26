#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNK_SIZE 1048576

int runningThreadsNum = 0, visitedThreadNum = 0, outputFileDesc = 0, totalBytesWritten = 0, curChunkSize = 0;
char globalChunkBuffer[CHUNK_SIZE];
pthread_mutex_t write_mutex;
pthread_cond_t count_threshold_cv;

void *threadFileRead(void *threadFileName)
{
    int readFileDesc, bytesRead;
    char *fileName = (char *)threadFileName;
    char threadChunkBuffer[CHUNK_SIZE];
    readFileDesc = open(fileName, O_RDONLY);
    if (readFileDesc < 0)
    {
        printf("Could not open file: filename = %s. %s\n", fileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = read(readFileDesc, threadChunkBuffer, CHUNK_SIZE)) >= 0)
    {
        if (pthread_mutex_lock(&write_mutex) < 0)
        {
            printf("pthread_mutex_lock error. %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        //got the lock
        if (bytesRead < CHUNK_SIZE) //remember to check if bytesRead is smaller than chunkSize. If so, runningThreadsNum--.
            runningThreadsNum--;
        else
            visitedThreadNum++;
    }
    if (bytesRead < 0)
    {
        printf("Could not read from file: filename = %s. %s\n", fileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t *thread_ids;
    int i = 0;
    runningThreadsNum = argc - 2;
    printf("Hello, creating %s from %d input files", argv[1], runningThreadsNum);
    outputFileDesc = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outputFileDesc < 0)
    {
        printf("Could not create or open file: filename = %s. %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    thread_ids = (pthread_t *)malloc(sizeof(pthread_t) * runningThreadsNum);
    if (thread_ids == NULL)
    {
        printf("Could not create threads array. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < runningThreadsNum; i++)
    {
        if (pthread_create(&thread_ids[i], NULL, threadFileRead, (void *)argv[i + 2]) < 0)
        {
            printf("Could not create thread. %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < argc - 2; i++)
    {
        if (pthread_join(thread_ids[i], NULL) < 0)
        {
            printf("pthread_join error. %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    free(thread_ids);
    close(outputFileDesc);
    printf("Created %s with size %d bytes", argv[1], totalBytesWritten);
    exit(EXIT_SUCCESS);
}