#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>

int *allChildPids;
int childNum=0;
char *symCountPath;
int (*allChildPipes)[2];


void freeMemResources() {
	if (allChildPids != NULL)
		free(allChildPids);
	if (symCountPath != NULL)
		free(symCountPath);
	if (allChildPipes != NULL)
		free(allChildPipes);
}

void handleForkError(int errPlace) {
	int i = 0;
	for (i = 0; i < errPlace; i++) {
		close(allChildPipes[i][0]);
		//close(allChildPipes[i][1]); write end already closed (see 'else' in fork iteration)
	}
}

int findChildIndex(int pid, int* childArray, int arraySize) {
	int i = 0;
	for (i = 0; i < arraySize; i++) {
		if (childArray[i] == pid)
			break;
	}
	return i;
}

void my_signal_handler(int signum, siginfo_t* info, void* ptr) {
	int i = 0;
	if (signum == SIGPIPE) {
		printf("SIGPIPE for Manager process %d. Leaving.\n", getpid());
		for (i = 0; i < childNum; i++)
			kill(allChildPids[i], SIGTERM);
		freeMemResources();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char* argv[]) {
	int i = 0, pid = 0, curChild = 0, exitCode = 0, curChildIndex = -1;
	char *dirpath;
	char letterString[2] = "\0";
	char childOutputPipeString[2] = "\0";
	struct sigaction new_action;
	childNum = strlen(argv[2]);
	dirpath = dirname(argv[0]);
	allChildPids = (int*)malloc(childNum * sizeof(int));
	if (allChildPids == NULL) {
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	symCountPath = (char*)malloc((strlen(dirpath) + strlen("/sym_count") + 1) * sizeof(char));
	if (symCountPath == NULL) {
		freeMemResources();
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	strcpy(symCountPath, dirpath);
	strcat(symCountPath, "/sym_count");

	allChildPipes = (int**)malloc(childNum * sizeof(int) * 2);
	if (allChildPipes == NULL) {
		freeMemResources();
		printf("malloc error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_sigaction = my_signal_handler;
	new_action.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGPIPE, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		freeMemResources();
		exit(EXIT_FAILURE);
	}


	for (i = 0; i < childNum; i++) {
		if (pipe(allChildPipes[i]) == -1) {
			printf("pipe error: %s\n", strerror(errno));
			freeMemResources();
			handleForkError(i);
			exit(EXIT_FAILURE);
		}
		pid = fork();
		if (pid < 0) {
			printf("fork error: %s\n", strerror(errno));
			freeMemResources();
			handleForkError(i);
			exit(EXIT_FAILURE);
		}
		else if (pid == 0) {
			close(allChildPipes[i][0]); //child: close unsued read end
			letterString[0] = argv[2][i];
			childOutputPipeString[0] = allChildPipes[i][1]; //child pipe write descriptor
			char* childArgs[] = { symCountPath, argv[1],letterString ,childOutputPipeString,NULL };
			if (execvp(symCountPath, childArgs) == -1) {
				printf("execvp failed. program path: %s\targv[0]=%s, argv[1]=%s, argv[2]=%s, argv[2]=%s\n", symCountPath, childArgs[0], childArgs[1], childArgs[2], childArgs[3]);
				return 0;
			}
		}
		else {
			allChildPids[i] = pid;
			close(allChildPipes[i][1]); //parent: close unused write end
		}

	}
	while (-1 != (curChild = wait(&exitCode))) {
		sleep(1);
		curChildIndex = findChildIndex(curChild, allChildPids, childNum);
		if (!WIFEXITED(exitCode))
			printf("child pid=%d terminated abnormally with exit code=%d. continuing anyway.\n", allChildPids[curChildIndex], exitCode);
		else {
			//read from pipe the child's output into string (char array)
			//printf said string
			//close pipe's read end
		}
		allChildPids[curChildIndex] = allChildPids[childNum - 1];
		allChildPipes[curChildIndex] = allChildPipes[childNum - 1];
		if ((childNum - 1) == 0) {
			free(allChildPids);
			free(allChildPipes);
		}

		else {
			allChildPids = (int*)realloc(allChildPids, sizeof(int)*(childNum - 1));
			allChildPipes = (int**)realloc(allChildPids, 2 * sizeof(int)*(childNum - 1));
		}

		childNum--;
	}
	freeMemResources();
	exit(EXIT_SUCCESS);
}