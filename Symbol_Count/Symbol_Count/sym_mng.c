int main(int argc, char* argv[]) {
	int i = 0,tbound=argv[3][0]-'0',pid=0;
	for (i = 0; i < strlen(argv[2]); i++) {
		pid = fork();
		if (pid < 0) {
			printf("ERROR: foek() failed");
			exit(0);
		}
		if (pid == 0) {
			//child
		}
	}
}