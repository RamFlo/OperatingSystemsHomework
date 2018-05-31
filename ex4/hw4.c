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
char globalChunkBuffer[CHUNK_SIZE] = "";
pthread_mutex_t write_mutex;
pthread_cond_t prev_iter_finished_cv;

void *threadFileRead(void *threadFileName)
{
    int readFileDesc, bytesRead, i = 0;
    char *fileName = (char *)threadFileName;
    char threadChunkBuffer[CHUNK_SIZE] = "";
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
        if (bytesRead < CHUNK_SIZE) //check if bytesRead is smaller than chunkSize. If so, runningThreadsNum--.
            runningThreadsNum--;
        else
            visitedThreadNum++;
        if (bytesRead > curChunkSize) //updating current chunk size, if needed
            curChunkSize = bytesRead;
        for (i = 0; i < bytesRead; i++) //XOR globalChunkBuffer
            globalChunkBuffer[i] ^= threadChunkBuffer[i];
        if (runningThreadsNum == visitedThreadNum) //this is the last thread for this chunk
        {
            if (write(outputFileDesc, globalChunkBuffer, curChunkSize) != curChunkSize) //update output file
            {
                printf("Could not write to output file. %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            totalBytesWritten += curChunkSize;
            curChunkSize = 0;
            visitedThreadNum = 0;
            for (i = 0; i < CHUNK_SIZE; i++)
                globalChunkBuffer[i] = 0;
            if (pthread_cond_broadcast(&prev_iter_finished_cv) != 0)
            {
                printf("Could not broadcast on signal. %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else if (bytesRead == CHUNK_SIZE) //not last thread and not finished: wait for iteration to finish
        {
            if (pthread_cond_wait(&prev_iter_finished_cv, &write_mutex) != 0)
            {
                printf("Could not wait on condition. %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (pthread_mutex_unlock(&write_mutex) != 0)
        {
            printf("Could not unlock mutex. %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (bytesRead < CHUNK_SIZE)
            break;
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
    printf("Hello, creating %s from %d input files\n", argv[1], runningThreadsNum);
    outputFileDesc = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outputFileDesc < 0)
    {
        printf("Could not create or open file: filename = %s. %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Initialize mutex and condition variable objects
    if (pthread_mutex_init(&write_mutex, NULL) != 0)
    {
        printf("Could not initiate mutex. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&prev_iter_finished_cv, NULL) != 0)
    {
        printf("Could not initiate condition. %s\n", strerror(errno));
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
    printf("Created %s with size %d bytes\n", argv[1], totalBytesWritten);
    exit(EXIT_SUCCESS);
}