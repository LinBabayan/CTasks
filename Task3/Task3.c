#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/wait.h>
#include <sys/types.h>

void errorHandle(char* errorText, int p[2]){
	printf("%s\n", errorText);
	close(p[0]);
	close(p[1]);
	exit(EX_OSERR);
}

void childProc1(const char *prog1){
	//child 1 prog1.sh
	execlp(prog1, prog1, (char*) NULL);
	perror("prog1 failed\n");
	exit(EX_OSERR);
}

void childProc2(const char *prog2, int p[2]){
	//child 2 prog2.sh
	//redirect stdout in pipe

	if(dup2(p[1], STDOUT_FILENO) < 0){
		errorHandle("output redirection error", p);
	}
	close(p[0]);
	close(p[1]);

	execlp(prog2, prog2, (char *)NULL);
	perror("prog2 failed\n");
	exit(EX_OSERR);
}

void childProc3(const char *prog3, int p[2], const char *filename){
	//child 3 prog3.sh
	//redirect stdin to be from pipe, open file, redirect output in file, execute prog3
	
	if(dup2(p[0], STDIN_FILENO) < 0){ //input from pipe
		errorHandle("input redirection error", p);		
	}
	close(p[1]);
	close(p[0]);
	
	int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); //rw-r--r--
	if(file < 0){
		perror("file open failure\n");
		exit(EX_OSERR);
	}
	
	if(dup2(file, STDOUT_FILENO) < 0){//otput to file
		perror("file output error\n");
		close(file);
		exit(EX_OSERR);
	}
	close(file);
	
	execlp(prog3, prog3, (char*)NULL);
	perror("prog3 failed\n");
	exit(EX_OSERR);		
}

int main(int argc, char *argv[]){
	if(argc != 5){
		perror("wrong argument number\n");
		return 1;
	}
	
	pid_t pids[3];
	int p[2], status;
	
	switch (pids[0] = fork()) {
		case 0:
			childProc1(argv[1]);
			
		case -1:
			perror("fork1 error\n");
			exit(EX_OSERR);
	}
	
	waitpid(pids[0], &status, 0);
	
	if(WIFEXITED(status)){
		int exit_code = WEXITSTATUS(status);
		if(exit_code){
			perror("prog1 filed\n");
			exit(EX_OSERR);
		}
	}
	else{
		perror("prog1 failed\n");
		exit(EX_OSERR);
	}
	
	if(pipe(p) < 0){
		perror("pipe error\n");
		exit(EX_OSERR);
	}
	
	switch (pids[1] = fork()) {
		case 0:
			childProc2(argv[2], p);

		case -1:
			errorHandle("fork2 error", p); 
	}
	
	switch (pids[2] = fork()){
		case 0:
			childProc3(argv[3], p, argv[4]);
		case -1:
			errorHandle("fork2 error", p);
	}
	
	close(p[0]);
	close(p[1]);
	wait(NULL);
	
	return 0;
}
