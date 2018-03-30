#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

char sym = '\0';
int counter = 0;

void my_signal_handler(int signum, siginfo_t* info, void* ptr) {
	if (signum == SIGCONT)
		printf("Process %d continues\n", (int)info->si_pid);
	if (signum == SIGTERM) {
		printf("Process %d finishes. Symbol %c. Instances %d.\n", (int)info->si_pid,sym,counter);
		exit(0);
	}
}

int main(int argc, char* argv[]) {
	FILE* myFile;
	char buffer[1024];
	int i=0;
	myFile = fopen(argv[1], "r");
	sym = argv[2][0];
	struct sigaction new_action;
	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_sigaction = my_signal_handler;
	new_action.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGCONT, &new_action, NULL) || 0 != sigaction(SIGTERM, &new_action, NULL)) {
		printf("Signal handle registration " "failed. %s\n", strerror(errno));
		return -1;
	}
	if (myFile == NULL) {
		printf("ERROR: could not open file");
		exit(0);
	}
	while (fgets(buffer, 1024, myFile) != NULL) {
		for (i = 0; i < strlen(buffer); i++) {
			if (buffer[i] == sym) {
				counter++;
				printf("Process %d, symbol %c, going to sleep\n", getpid(), sym);
				raise(SIGSTOP);
			}
		}
	}
	raise(SIGTERM);
}