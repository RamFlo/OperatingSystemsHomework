#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

char symbol = '\0';
int counter = 0;
int fileDesc = 0;

void my_signal_handler(int signum, siginfo_t* info, void* ptr) {
	if (signum == SIGCONT)
		printf("Process %d continues\n", getpid());
	if (signum == SIGTERM) {
		printf("Process %d finishes. Symbol %c. Instances %d.\n", getpid(), symbol,counter);
		if (close(fileDesc) != 0)
			printf("error closing file %s\n", strerror(errno));
		exit(0);
	}
}

int main(int argc, char* argv[]) {
	char buffer[BUFFER_SIZE];
	int i=0,charsRead=0;
	symbol = argv[2][0];
	fileDesc = open(argv[1], O_RDONLY);
	struct sigaction new_action;
	if (fileDesc <0) {
		printf("Could not open file: filename = %s. %s\n", argv[1], strerror(errno));
		return -1;
	}
	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_sigaction = my_signal_handler;
	new_action.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGCONT, &new_action, NULL) || 0 != sigaction(SIGTERM, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		return -1;
	}
	while ((charsRead=read(fileDesc,buffer, BUFFER_SIZE))!=0) {
		if (charsRead < 0) {
			printf("error reading from file %s\n", strerror(errno));
			if (close(fileDesc)!=0)
				printf("error closing file %s\n", strerror(errno));
			return -1;
		}
		for (i = 0; i < charsRead; i++) {
			if (buffer[i] == symbol) {
				counter++;
				printf("Process %d, symbol %c, going to sleep\n", getpid(), symbol);
				raise(SIGSTOP);
			}
		}
	}
	raise(SIGTERM);
}