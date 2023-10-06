#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

int child_received_signal = 0; // A flag to indicate if the child received a signal
int parent_received_signal = 0;

/*void child_signal_handler(int signo) {
    printf("Child process received signal: %s\n", strsignal(signo));
    child_received_signal = 1;
}

void parent_signal_handler(int signo) {
    printf("Parent process received signal: %s\n", strsignal(signo));
    child_received_signal = 1;
} */


const char* signal_names[] = {// Define an array of signal names
    "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
    "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1",
    "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM",
    "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
    "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ",
    "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR",
    "SIGSYS"
};

/* void signal_handler(int signo) {
	child_received_signal = 1;
    printf("Child process received signal: ");
    switch (signo) {
		case SIGABRT:
			printf("SIGABRT\n");
            break;
		case SIGALRM:
			printf("SIGALRM\n");
            break;
		case SIGBUS:
			printf("SIGBUS\n");
            break;
		case SIGFPE:
			printf("SIGFPE\n");
            break;
		case SIGHUP:
			printf("SIGHUP\n");
            break;
		case SIGILL:
			printf("SIGILL\n");
            break;
		case SIGINT:
			printf("SIGINT\n");
            break;
		case SIGKILL:
			printf("SIGKILL\n");
            break;
		case SIGPIPE:
			printf("SIGPIPE\n");
            break;
		case SIGQUIT:
			printf("SIGQUIT\n");
            break;
		case SIGSEGV:
			printf("SIGSEGV\n");
            break;
		case SIGSTOP:
			printf("SIGSTOP\n");
            break;
		case SIGTERM:
			printf("SIGTERM\n");
            break;
		case SIGTRAP:
			printf("SIGTRAP\n");
            break;

        // Add more signal cases as needed
        default:
            printf("Unknown signal\n");
    }
} */

int main(int argc, char *argv[]){

	

	if (argc < 2) {
        fprintf(stderr, "Usage: %s <program_to_execute> [arg1] [arg2] ...\n", argv[0]);
        return 1;
    }

	char* program = argv[1];
	int status;

	/* fork a child process */
	pid_t pid;
	printf("Progress start to fork.\n");
	//int status = 0;
	pid = fork();
	
	/* execute test program */ 
	if (pid == 0){


		printf("I'm the Child Process, my pid = %d \n", getpid());
		printf("Child process start to execute test program: \n");

		/* for (int signo = 1; signo <= _NSIG; signo++) {
            signal(signo, child_signal_handler);
        } */
 
		/* char *args[argc];
		for (int i = 0; i < argc-1; i++) {
			args[i - 1] = argv[i];
		}

		args[argc - 1] = NULL; 
		execve(args[0], args, NULL); */
		execvp(argv[1], &argv[1]);
/* 
		if (execvp(program, args) == -1) {
            perror("execvp");
            exit(1);
        } */
		// If execvp fails
        perror("execve");
        exit(EXIT_FAILURE);
	}
	
	/* wait for child process terminates */
	else if (pid > 0){
		printf("I'm the Parent Process, my pid = %d \n", getpid());
		//pid = wait(&status);
		
        waitpid(pid, &status, WUNTRACED);

		/* for (int signo = 1; signo <= _NSIG; signo++) {
            signal(signo, parent_signal_handler);
        } */
		printf("Parent process receives SIGCHLD signal.\n");

		if (WIFEXITED(status)) {
            printf("Normal termination with EXIT STATUS = %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
			int signum = WTERMSIG(status);
			if (signum >0 && signum < sizeof(signal_names) / sizeof(signal_names[0])){
				printf("Child process get %s signal.\n", signal_names[signum - 1]);
			}else{

				printf("Child process get %d signal.\n", WTERMSIG(status));
			}
        } else if (WIFSTOPPED(status)) {
			int signum = WSTOPSIG(status);
			if (signum >= 0 && signum < sizeof(signal_names) / sizeof(signal_names[0])) {
                printf("Child process get %s signal.\n", signal_names[signum - 1]);
            } else {
				printf("Child process get %d signal.\n", WSTOPSIG(status));
                
            }

		}
	}
	else {
		printf("fork error.\n");
	}
	/* check child process'  termination status */
	
	
}
