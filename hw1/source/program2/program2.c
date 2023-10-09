#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>
#include <linux/fs.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
#define UMH_NO_WAIT     0       /* don't wait at all */
#define UMH_WAIT_EXEC   1       /* wait for the exec, but not the process */
#define UMH_WAIT_PROC   2       /* wait for the process to complete */
#define UMH_KILLABLE    4       /* wait for EXEC/PROC killable */

static struct task_struct *task;
/* struct kernel_clone_args {
    u64 flags;
    int __user *pidfd;
    int __user *child_tid;
    int __user *parent_tid;
    int exit_signal;
    unsigned long stack;
    unsigned long stack_size;
    unsigned long tls;
    pid_t *set_pid;
    size_t set_tid_size;
    int cgroup;
    struct cgroup *cgrp;
    struct css_set *cset;
}; */
struct wait_opts {
	enum pid_type wo_type;
	int	wo_flags;
	struct pid *wo_pid;

	struct waitid_info *wo_info;
	int wo_stat;
	struct rusage *wo_rusage;

	wait_queue_entry_t child_wait;
	int notask_error;
};

extern pid_t kernel_clone(struct kernel_clone_args *kargs);
extern int do_execve( struct filename *filename,
	 				  const char __user *const __user *__argv,
					  const char __user *const __user *__envp);
extern struct filename *getname_kernel(const char *filename);
extern long do_wait(struct wait_opts *wo);


void my_wait(pid_t pid){
	int status;
	struct wait_opts wo;
	struct pid *wo_pid=NULL;
	enum pid_type type;
	type = PIDTYPE_PID;
	wo_pid = find_get_pid(pid);

	wo.wo_type=type;
	wo.wo_pid=wo_pid;
	wo.wo_flags=WEXITED | WUNTRACED;
	wo.wo_info=NULL;
	//wo.wo_stat=(int __user*)&status;
	wo.wo_stat=status;
	wo.wo_rusage=NULL;

    
	//printk("[program2] : my_wait() is executed.");
	do_wait(&wo);//error happens here and the module stopped.
	long signal = wo.wo_stat&127;
	if (signal < 0){
		printk("do_wait is error with signal %d", signal);
	}
	//printk("do_wait return value is %d\n", &signal);
	//printk("do_wait return value is %d\n", signal);

    //printk("[Do_Fork] : The return signal is %d\n", wo.wo_stat);

	const char* signal_names[] = {// Define an array of signal names
    "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
    "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1",
    "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM",
    "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
    "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ",
    "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR",
    "SIGSYS"
};
	int ifexit = ((((signal)) & 0x7f) == 0);//WIFEXITED
	int exitstatus = ((((signal)) & 0xff00) >> 8); //WEXITSTATUS
	bool ifsig = (((signed char) ((((signal)) & 0x7f) + 1) >> 1) > 0); //WIFSIGNALED
	int sign = signal & 0x7f; // WTERMSIG
	int stop = ((((signal)) & 0xff) == 0x7f); // WIFSTOPPED
	int stopsig = ((((signal)) & 0xff00) >> 8);//WSTOPSIG

	if (ifexit) {
		printk("[program2] : Normal termination with EXIT STATUS = %d\n", exitstatus);
		printk("[program2] : child process terminated.");
		printk("[program2] : The return signal is %d\n", signal);
	} else if (ifsig) {
		int signum = sign;
		if (signum >0 && signum < sizeof(signal_names) / sizeof(signal_names[0])){
			printk("[program2] : get %s signal.\n", signal_names[signum - 1]);
			printk("[program2] : child process terminated.");
			printk("[program2] : The return signal is %d\n", signal);
		}else{

			printk("[program2] : get %d signal.\n", sign);
			printk("[program2] : child process terminated.");
			printk("[program2] : The return signal is %d\n", signal);
		}
	} else if (stop) {
		int signum = 19+stopsig;
		//printk("[program2] : stop = %d, stopsig = %d", stop, stopsig);
		if (signum >= 0 && signum < sizeof(signal_names) / sizeof(signal_names[0])) {
			printk("[program2] : get %s signal.\n", signal_names[signum - 1]);
			printk("[program2] : child process terminated.");
			printk("[program2] : The return signal is %d\n", signum);
		} else {
			printk("[program2] : get %d signal.\n", stopsig);
			printk("[program2] : child process terminated.");
			printk("[program2] : The return signal is %d\n", signal);
			
		}

	}

	put_pid(wo_pid); // decrease the count and free memory

	return;
}

int my_exec(void){
	int result;

	/* const char *path = "/home/vagrant/csc3150/CSC3150/hw1/source/program2/test";
	const char *const argv[] = {path, NULL, NULL};
	const char *const envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL }; */
	//char path[] = "/home/vagrant/csc3150/CSC3150/hw1/source/program2/test";
	char path[] = "/tmp/test";
	char *argv[] = {path, NULL, NULL};
	char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	//printk("[program2] : i don't think my_wait() has any problems.");
	result = 0;
	struct filename *file = getname_kernel(path);
	if (IS_ERR(file)) {
            printk(KERN_ERR "[program2] : Failed to get executable path\n");
            return 0;
        }

	printk( "[program2] : child process");
	result = do_execve(file, NULL, NULL);
	//result = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

	if (result < 0) {
        printk(KERN_ERR "[program2] : do_execve failed with error code %d\n", result);
        return result; // Return the error code
    }
	else{
		//printk(KERN_INFO "[program2] : result is %d\n", result);
		//printk(KERN_INFO "[program2] : result&127 is %d\n", result&127);
		return 0;
	}

	do_exit(result);
}

//implement fork function
int my_fork(void *argc){
	
	
	//set default sigaction for current process
	int i;
	struct k_sigaction *k_action = &current->sighand->action[0];
	for(i=0;i<_NSIG;i++){
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	
	/* fork a process using kernel_clone or kernel_thread */
	pid_t child_pid;

    struct kernel_clone_args args = {
        .flags = ((SIGCHLD | CLONE_VM | CLONE_UNTRACED) & ~CSIGNAL),
		.pidfd = NULL,
        .parent_tid = NULL,
        .child_tid = NULL,
        .exit_signal = (SIGCHLD & CSIGNAL),
        .stack = (unsigned long)&my_exec,
        .stack_size = 0,
        .tls = 0,
    };

	child_pid = kernel_clone(&args);
	if(child_pid < 0){
		printk(KERN_ERR "[program2] : fork error\n");
		return -1;
	}

	printk(KERN_INFO "[program2] : The Child process has pid = %d\n", child_pid);
	printk(KERN_INFO "[program2] : This is the parent process, pid = %d\n", (int)current->pid);
	
	/* execute a test program in child process */
	
	/* wait until child process terminates */
	my_wait(child_pid);
	return 0;
}

static int __init program2_init(void){

	printk("[program2] : Module_init\n");
	
	/* write your code here */
    printk("[program2] : module_init create kthread start\n");
    task = kthread_create(&my_fork, NULL, "Mythread");

	//wake up new thread if ok
	if(!IS_ERR(task)){
		printk("[program2] : module_init Kthread starts\n");
		wake_up_process(task);
	}


	/* create a kernel thread to run my_fork */
	
	return 0;
}

static void __exit program2_exit(void){
	printk("[program2] : Module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
