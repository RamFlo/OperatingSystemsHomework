#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>

int findChildIndex(int pid, int* childArray,int arraySize) {
	int i = 0;
	for (i = 0; i < arraySize; i++) {
		if (childArray[i] == pid)
			break;
	}
	return i;
}

int main(int argc, char* argv[]) {
	int i = 0,tBound=argv[3][0]-'0',pid=0,childNum=0,curChild=0,exitCode=0,curChildIndex=-1;
	if (tBound <= 0)
		return 0;
	int *allChildPids, *allChildStopCounts;
	char *symCountPath,*dirpath;
	char letterString[2] = "\0";
	childNum = strlen(argv[2]);
	dirpath = dirname(argv[0]);
	allChildPids = (int*)malloc(childNum * sizeof(int));
	if (allChildPids == NULL) {
		printf("malloc error: %s\n", strerror(errno));
		return -1;
	}
	allChildStopCounts= (int*)malloc(childNum * sizeof(int));
	if (allChildPids == NULL) {
		free(allChildPids);
		printf("malloc error: %s\n", strerror(errno));
		return -1;
	}
	symCountPath = (char*)malloc((strlen(dirpath) + strlen("/sym_count") + 1) * sizeof(char));
	if (symCountPath == NULL) {
		free(allChildPids);
		free(allChildStopCounts);
		printf("malloc error: %s\n", strerror(errno));
		return -1;
	}
	strcpy(symCountPath, dirpath);
	strcat(symCountPath, "/sym_count");
	for (i = 0; i < childNum; i++) {
		pid = fork();
		if (pid < 0) {
			printf("fork error: %s\n", strerror(errno));
			free(allChildPids);
			free(allChildStopCounts);
			free(symCountPath);
			return -1;
		}
		else if (pid == 0) {
			letterString[0] = argv[2][i];
			char* childArgs[] = { symCountPath, argv[1],letterString ,NULL};
			if (execvp(symCountPath, childArgs) == -1) {
				printf("execvp failed. program path: %s\targv[0]=%s, argv[1]=%s, argv[2]=%s\n", symCountPath, childArgs[0], childArgs[1], childArgs[2]);
				return 0;
			}
		}
		else
			allChildPids[i] = pid;
	}
	while (-1!=(curChild=waitpid(-1,&exitCode,WUNTRACED))) {
		sleep(1);
		curChildIndex=findChildIndex(curChild,allChildPids,childNum);
		if (WIFSTOPPED(exitCode)) {
			allChildStopCounts[curChildIndex]++;
			if (allChildStopCounts[curChildIndex] == tBound)
				kill(allChildPids[curChildIndex], SIGTERM);
			kill(allChildPids[curChildIndex], SIGCONT);
		}
		else {
			if (!WIFEXITED(exitCode)) {
				printf("child pid=%d terminated abnormally with exit code=%d. continuing anyway.\n", allChildPids[curChildIndex], exitCode);
			}
			allChildPids[curChildIndex] = allChildPids[childNum - 1];
			allChildStopCounts[curChildIndex] = allChildStopCounts[childNum - 1];
			if ((childNum - 1) == 0) {
				free(allChildPids);
				free(allChildStopCounts);
			}
			else {
				allChildPids = (int*)realloc(allChildPids, sizeof(int)*(childNum - 1));
				allChildStopCounts = (int*)realloc(allChildStopCounts, sizeof(int)*(childNum - 1));
			}
			childNum--;
		}
	}
	free(symCountPath);
	if (childNum != 0) {
		free(allChildPids);
		free(allChildStopCounts);
	}
}