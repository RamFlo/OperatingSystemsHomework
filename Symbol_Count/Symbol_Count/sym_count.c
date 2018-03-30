#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	FILE* myFile;
	char buffer[1024];
	int counter = 0,i=0;
	myFile = fopen(argv[1], "r");
	if (myFile == NULL) {
		printf("ERROR: could not open file");
		exit(0);
	}
	while (fgets(buffer, 1024, myFile) != NULL) {
		for (i = 0; i < strlen(buffer); i++) {
			if (buffer[i] == argv[2][0]) {
				counter++;
				printf("Process %d, symbol %c, going to sleep\n", 1, argv[2][0]);
			}
		}
	}
	//printf("%d", counter);
	return 0;
}