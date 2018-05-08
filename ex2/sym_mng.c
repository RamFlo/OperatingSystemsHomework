#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>

#define MAX_MSG_SIZE 1024

int *allChildPids;
int childNum = 0;
char *symCountPath;
int (*allChildPipes)[2];

void freeMemResources()
{
<<<<<<< HEAD
	if (allChildPids != NULL){
=======
<<<<<<< HEAD
    if (allChildPids != NULL)
        free(allChildPids);
    if (symCountPath != NULL)
        free(symCountPath);
    if (allChildPipes != NULL)
        free(allChildPipes);
=======
	if (allChildPids != NULL)
>>>>>>> c4ce13dc3cb7e5755df9afcc41f57c2500eceba5
		free(allChildPids);
		allChildPids=NULL;
	}
		
	if (symCountPath != NULL){
		free(symCountPath);
		symCountPath=NULL;
	}
		
	if (allChildPipes != NULL){
		free(allChildPipes);
<<<<<<< HEAD
		allChildPipes=NULL;
	}
		
=======
>>>>>>> 83c9764e1a7373e933d6e0a34ab05f9207a0ee27
>>>>>>> c4ce13dc3cb7e5755df9afcc41f57c2500eceba5
}

void handleForkError(int errPlace)
{
<<<<<<< HEAD
    int i = 0;
    for (i = 0; i < errPlace; i++)
    {
        close(allChildPipes[i][0]);
        //close(allChildPipes[i][1]); write end already closed (see 'else' in fork iteration)
    }
=======
	int i = 0;
	for (i = 0; i < errPlace; i++)
	{
		close(allChildPipes[i][0]);
		//close(allChildPipes[i][1]); write end already closed (see 'else' in fork iteration)
	}
>>>>>>> 83c9764e1a7373e933d6e0a34ab05f9207a0ee27
}

int findChildIndex(int pid, int *childArray, int arraySize)
{
<<<<<<< HEAD
    int i = 0;
    for (i = 0; i < arraySize; i++)
    {
        if (childArray[i] == pid)
            break;
    }
    return i;
=======
	int i = 0;
	for (i = 0; i < arraySize; i++)
	{
		if (childArray[i] == pid)
			break;
	}
	return i;
>>>>>>> 83c9764e1a7373e933d6e0a34ab05f9207a0ee27
}

void my_signal_handler(int signum, siginfo_t *info, void *ptr)
{
<<<<<<< HEAD
    int i = 0;
    if (signum == SIGPIPE)
    {
        printf("SIGPIPE for Manager process %d. Leaving.\n", getpid());
        for (i = 0; i < childNum; i++)
            kill(allChildPids[i], SIGTERM);
        freeMemResources();
        exit(EXIT_FAILURE);
    }
=======
	int i = 0;
	if (signum == SIGPIPE)
	{
		printf("SIGPIPE for Manager process %d. Leaving.\n", getpid());
		for (i = 0; i < childNum; i++)
			kill(allChildPids[i], SIGTERM);
		freeMemResources();
		exit(EXIT_FAILURE);
	}
>>>>>>> 83c9764e1a7373e933d6e0a34ab05f9207a0ee27
}

int main(int argc, char *argv[])
{
<<<<<<< HEAD
    int i = 0, pid = 0, curChild = 0, exitCode = 0, curChildIndex = -1;
    char *dirpath;
    char letterString[2] = "\0";
    char childOutputPipeString[2] = "\0";
    struct sigaction new_action;
    childNum = strlen(argv[2]);
    dirpath = dirname(argv[0]);
    allChildPids = (int *)malloc(childNum * sizeof(int));
    if (allChildPids == NULL)
    {
        printf("malloc error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    symCountPath = (char *)malloc((strlen(dirpath) + strlen("/sym_count") + 1) * sizeof(char));
    if (symCountPath == NULL)
    {
        freeMemResources();
        printf("malloc error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    strcpy(symCountPath, dirpath);
    strcat(symCountPath, "/sym_count");

    allChildPipes = (int **)malloc(childNum * sizeof(int) * 2);
    if (allChildPipes == NULL)
    {
        freeMemResources();
        printf("malloc error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_sigaction = my_signal_handler;
    new_action.sa_flags = SA_SIGINFO;
    if (0 != sigaction(SIGPIPE, &new_action, NULL))
    {
        printf("Signal handle registration failed. %s\n", strerror(errno));
        freeMemResources();
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < childNum; i++)
    {
        if (pipe(allChildPipes[i]) == -1)
        {
            printf("pipe error: %s\n", strerror(errno));
            freeMemResources();
            handleForkError(i);
            exit(EXIT_FAILURE);
        }
        pid = fork();
        if (pid < 0)
        {
            printf("fork error: %s\n", strerror(errno));
            freeMemResources();
            handleForkError(i);
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            close(allChildPipes[i][0]); //child: close unsued read end
            letterString[0] = argv[2][i];
            childOutputPipeString[0] = allChildPipes[i][1]; //child pipe write descriptor
            char *childArgs[] = {symCountPath, argv[1], letterString, childOutputPipeString, NULL};
            if (execvp(symCountPath, childArgs) == -1)
            {
                printf("execvp failed. program path: %s\targv[0]=%s, argv[1]=%s, argv[2]=%s, argv[2]=%s\n", symCountPath, childArgs[0], childArgs[1], childArgs[2], childArgs[3]);
                return 0;
            }
        }
        else
        {
            allChildPids[i] = pid;
            close(allChildPipes[i][1]); //parent: close unused write end
        }
    }
    while (-1 != (curChild = wait(&exitCode)))
    {
        sleep(1);
        curChildIndex = findChildIndex(curChild, allChildPids, childNum);
        if (!WIFEXITED(exitCode))
            printf("child pid=%d terminated abnormally with exit code=%d. continuing anyway.\n", allChildPids[curChildIndex], exitCode);
        else
        {
            //read from pipe the child's output into string (char array)
            //printf said string
            //close pipe's read end
        }
        allChildPids[curChildIndex] = allChildPids[childNum - 1];
        allChildPipes[curChildIndex] = allChildPipes[childNum - 1];
        if ((childNum - 1) == 0)
        {
            free(allChildPids);
            free(allChildPipes);
        }

        else
        {
            allChildPids = (int *)realloc(allChildPids, sizeof(int) * (childNum - 1));
            allChildPipes = (int **)realloc(allChildPids, 2 * sizeof(int) * (childNum - 1));
        }

        childNum--;
    }
    freeMemResources();
    exit(EXIT_SUCCESS);
=======
	int i = 0, pid = 0, curChild = 0, exitCode = 0, curChildIndex = -1, charsRead = 0;
	char *dirpath;
	char letterString[2] = "\0";
	char childOutputPipeString[20] = "\0";
	char childOutputString[MAX_MSG_SIZE];
	struct sigaction new_action;
	childNum = strlen(argv[2]);
	dirpath = dirname(argv[0]);
	allChildPids = (int *)malloc(childNum * sizeof(int));
	if (allChildPids == NULL)
	{
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	symCountPath = (char *)malloc((strlen(dirpath) + strlen("/sym_count") + 1) * sizeof(char));
	if (symCountPath == NULL)
	{
		freeMemResources();
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	strcpy(symCountPath, dirpath);
	strcat(symCountPath, "/sym_count");

	allChildPipes = (int(*)[])malloc(childNum * sizeof(int) * 2);
	if (allChildPipes == NULL)
	{
		freeMemResources();
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_sigaction = my_signal_handler;
	new_action.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGPIPE, &new_action, NULL))
	{
		printf("Signal handle registration failed. %s\n", strerror(errno));
		freeMemResources();
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < childNum; i++)
	{
		if (pipe(allChildPipes[i]) == -1)
		{
			printf("pipe error: %s\n", strerror(errno));
			freeMemResources();
			handleForkError(i);
			exit(EXIT_FAILURE);
		}
		pid = fork();
		if (pid < 0)
		{
			printf("fork error: %s\n", strerror(errno));
			freeMemResources();
			handleForkError(i);
			exit(EXIT_FAILURE);
		}
		else if (pid == 0)
		{
			close(allChildPipes[i][0]); //child: close unsued read end
			letterString[0] = argv[2][i];
			sprintf(childOutputPipeString,"%d",allChildPipes[i][1]); //child pipe write descriptor
			//childOutputPipeString[0] = allChildPipes[i][1]; 
			char *childArgs[] = {symCountPath, argv[1], letterString, childOutputPipeString, NULL};
			if (execvp(symCountPath, childArgs) == -1)
			{
				printf("execvp failed. program path: %s\targv[0]=%s, argv[1]=%s, argv[2]=%s, argv[3]=%s\n", symCountPath, childArgs[0], childArgs[1], childArgs[2], childArgs[3]);
				close(allChildPipes[i][1]); //close child write end if child program could not be executed
				freeMemResources();
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			allChildPids[i] = pid;
			close(allChildPipes[i][1]); //parent: close unused write end
		}
	}

	/*for (i = 0; i < childNum; i++){
		printf("child %d pipe: read: %d, write: %d\n",i,allChildPipes[i][0],allChildPipes[i][1]);
	}*/

	while (-1 != (curChild = wait(&exitCode)))
	{
		sleep(1);
		curChildIndex = findChildIndex(curChild, allChildPids, childNum);
		if (!WIFEXITED(exitCode))
			printf("child pid=%d terminated abnormally with exit code=%d. continuing anyway.\n", allChildPids[curChildIndex], exitCode);
		else
		{
			//read from pipe the child's output into string (char array)
			//printf said string
			//close pipe's read end
			if ((charsRead = read(allChildPipes[curChildIndex][0], childOutputString, MAX_MSG_SIZE - 1)) < 0)
				printf("could not read message from child process=%d.\n", allChildPids[curChildIndex]);
			else
			{
				childOutputString[charsRead] = '\0';
				printf("%s", childOutputString);
			}
			close(allChildPipes[curChildIndex][0]); //close parent read end from curChild
		}
		allChildPids[curChildIndex] = allChildPids[childNum - 1];
		allChildPipes[curChildIndex][0] = allChildPipes[childNum - 1][0];
		allChildPipes[curChildIndex][1] = allChildPipes[childNum - 1][1];
		if ((childNum - 1) == 0)
		{
			free(allChildPids);
			free(allChildPipes);
		}
		else
		{
			allChildPids = (int *)realloc(allChildPids, sizeof(int) * (childNum - 1));
			allChildPipes = (int(*)[])realloc(allChildPipes, 2 * sizeof(int) * (childNum - 1));
		}
		childNum--;
	}
	if (childNum != 0)
		freeMemResources();
	else
		free(symCountPath);
	exit(EXIT_SUCCESS);
>>>>>>> 83c9764e1a7373e933d6e0a34ab05f9207a0ee27
}