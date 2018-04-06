#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>

int findChildIndex(int pid, int* childArray,int arraySize) {
	for (int i = 0; i < arraySize; i++) {
		if (childArray[i] == pid)
			return i;
	}
	return -1;
}

int main(int argc, char* argv[]) {
	int i = 0,tBound=argv[3][0]-'0',pid=0,childNum=0,curChild=0,exitCode=0,curChildIndex=-1;
	if (tBound <= 0)
		exit(0);
	int * childPids;
	int * childStopCounters;
	char* symCountPath;
	char letterString[2] = "\0";
	childNum = strlen(argv[2]);
	childPids = (int*)malloc(childNum*sizeof(int));//NULL check
	childStopCounters= (int*)malloc(childNum * sizeof(int));//NULL check and free first
	for (i = 0; i < childNum; i++) {
		pid = fork();
		if (pid < 0) {
			printf("ERROR: fork() failed");
			//free malloc
			exit(0);
		}
		else if (pid == 0) {
			letterString[0] = argv[2][i];
			strcpy(symCountPath, dirname(argv[0]));
			strcat(symCountPath, "/sym_count");
			//printf("sending filename: %s\n", argv[1]);
			char* childArgs[] = { symCountPath, argv[1],letterString ,NULL};
			if (execvp(symCountPath, childArgs) == -1) {
				printf("ERROR: execvp failed\n");
				printf("prog path: %s \n", symCountPath);
				//free malloc
				exit(0);
			}
		}
		else
			childPids[i] = pid;
	}
	while (-1!=(curChild=waitpid(-1,&exitCode,WUNTRACED))) {
		sleep(1);
		curChildIndex=findChildIndex(curChild,childPids,childNum);
		if (WIFSTOPPED(exitCode)) {
			childStopCounters[curChildIndex]++;
			if (childStopCounters[curChildIndex] == tBound)
				kill(childPids[curChildIndex], SIGTERM);
			kill(childPids[curChildIndex], SIGCONT);
		}
		else {
			childPids[curChildIndex] = childPids[childNum - 1];
			childStopCounters[curChildIndex] = childStopCounters[childNum - 1];
			childPids = (int*)realloc(childPids, sizeof(int)*(childNum - 1));
			childStopCounters = (int*)realloc(childStopCounters, sizeof(int)*(childNum - 1));
			childNum--;
		}
	}
	free(childPids);
	free(childStopCounters);
}