int main(int argc, char* argv[]) {
	int i = 0,tbound=argv[3][0]-'0',pid=0;
	for (i = 0; i < strlen(argv[2]); i++) {
		pid = fork();
		if (pid < 0) {
			printf("ERROR: foek() failed");
			exit(0);
		}
		//need to unserstand: what does "exclude the process from the list of managed processes" mean?
		else if (pid == 0) {
			//execute child program (sym_count) - check for error!
		}
		else {
			//parent code: waitpid for child and once child returned use SIGCONT, max of termination bound
			//when child process exited OR reached termination bound: SIGTERM the child
		}
	}
}