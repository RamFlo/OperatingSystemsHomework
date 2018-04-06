#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int search(int Pid, int* ArrayC, int size) {
	for (int i = 0; i < size; i++) {
		if (ArrayC[i] == Pid) {
			return i;
		}
	}
	return -1;
}
int main(int argsc, char * argv[]) {
	FILE * myF;
	int numOfChild = strlen(argv[2]);
	int * childrenP;
	int currchild = 0;
	int * counterChild;
	int childCurrentIndex = 0;
	int exitCode = 0;
	char S[2] = "\0";
	counterChild = (int*)malloc(numOfChild * sizeof(int));
	if (counterChild == NULL) {
		printf("Error in malloc");
		exit(0);
	}
	childrenP = (int*)malloc(numOfChild * sizeof(int));
	if (childrenP == NULL) {
		printf("Error in malloc");
		free(counterChild);
		exit(0);
	}
	int k, ter_b = argv[3][0] - '0';
	int pID = 0;
	if (ter_b <= 0) {
		free(counterChild);
		free(childrenP);
		exit(0);
	}
	for (k = 0; k < strlen(argv[2]); k++) {
		pID = fork();
		if (pID < 0) {
			printf("Error- fork failed");
			return 0;
		}
		else if (pID == 0) {
			S[0] = argv[2][k];
			char * child[] = { "./sym_count",argv[1],S,NULL };
			if (execvp("./sym_count", child) == -1) {
				printf("Error in execvp");
				exit(0);

			}
		}
		else {
			//printf("1\n");
			childrenP[k] = pID;
			//printf("2\n");
		}
	}
	while (-1 != (currchild = waitpid(-1, &exitCode, WUNTRACED))) {
		sleep(1);
		childCurrentIndex = search(currchild, childrenP, numOfChild);
		if (WIFSTOPPED(exitCode)) {
			counterChild[childCurrentIndex] ++;
			if (counterChild[childCurrentIndex] == ter_b) {
				kill(childrenP[childCurrentIndex], SIGTERM);
				kill(childrenP[childCurrentIndex], SIGCONT);
			}
			else {
				kill(childrenP[childCurrentIndex], SIGCONT);
			}
		}
		else {
			childrenP[childCurrentIndex] = childrenP[numOfChild - 1];
			counterChild[childCurrentIndex] = counterChild[numOfChild - 1];
			childrenP = (int*)realloc(childrenP, sizeof(int)*(numOfChild - 1));
			if (childrenP == NULL) {
				printf("Error in malloc");
				free(counterChild);
				free(childrenP);
				exit(0);
			}
			counterChild = (int*)realloc(counterChild, sizeof(int)*(numOfChild - 1));
			if (counterChild == NULL) {
				printf("Error in malloc");
				free(counterChild);
				free(childrenP);
				exit(0);
			}
			numOfChild--;
		}
		free(childrenP);
		free(counterChild);
	}
}