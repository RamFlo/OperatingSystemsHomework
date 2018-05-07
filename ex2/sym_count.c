#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

char symbol = '\0';
int counter = 0;
int fileDesc = 0;
int fileSize = 0;
char *readData;

int freeMemResources() {
	int exitCode = EXIT_SUCCESS;
	if (close(fileDesc) != 0) {
		printf("error closing file %s\n", strerror(errno));
		exitCode = EXIT_FAILURE;
	}
	if (munmap(readData, fileSize) == -1) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		exitCode = EXIT_FAILURE;
	}
	return exitCode;
}

void my_signal_handler(int signum, siginfo_t* info, void* ptr) {
	if (signum == SIGPIPE) {
		printf("SIGPIPE for process %d. Symbol %c. Counter %d. Leaving.\n", getpid(), symbol, counter);
		freeMemResources();
		exit(EXIT_FAILURE);
	}
	if (signum == SIGTERM){
        freeMemResources();
        exit(EXIT_FAILURE);
    }
		
}

int main(int argc, char* argv[]) {
	int i = 0, charsRead = 0;
	symbol = argv[2][0];
	fileDesc = open(argv[1], O_RDONLY);
	struct sigaction new_action;
	struct stat fileStat;
	if (fileDesc < 0) {
		printf("Could not open file: filename = %s. %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (stat(argv[1], &fileStat) < 0) {
		printf("Could not execute STAT on file: filename = %s. %s\n", argv[1], strerror(errno));
		if (close(fileDesc) != 0)
			printf("error closing file %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fileSize = fileStat.st_size;
	readData = (char*)mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fileDesc, 0);
	if (MAP_FAILED == readData) {
		printf("Error mmapping the file: %s\n", strerror(errno));
		if (close(fileDesc) != 0)
			printf("error closing file %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_sigaction = my_signal_handler;
	new_action.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGPIPE, &new_action, NULL) || 0 != sigaction(SIGTERM, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n", strerror(errno));
		freeMemResources();
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < fileSize; i++) {
		if (readData[i] == symbol)
			counter++;
	}

	//write sprintf into fd=argv[3][0]
	//close write end
	exit(freeMemResources());
}