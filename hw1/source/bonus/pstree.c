#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Process {
	char *name;
	int pid;
	int ppid;
	int pgid;
	struct Process *parent;
	struct Process *child[128];
	struct Process *next;
};
static struct Process *node_head;
struct Process *node_root;

void find_proc();
struct Process *find_parent(int ppid);
void link_node();
void find_root();
void version();
char *find_name(char *str, int start, int end);
int length(int data);
void print_tree(struct Process *root, int level);
void print_tree_p(struct Process *root, int level);
void print_tree_g(struct Process *root, int level);

void find_proc()
{
	struct Process *node;
	int pid = 0;
	int count = 0;
	struct dirent *entry;
	DIR *dp = opendir("/proc");

	while ((entry = readdir(dp)) != NULL) {
		pid = atoi(entry->d_name);
		if (pid == 0) {
			continue;
		} else {
			char filename[100];
			sprintf(filename, "/proc/%d/stat", pid);
			FILE *f = fopen(filename, "r");

			int unused1;
			char name_old[100];
			char state;
			int ppid;
			int pgid;

			fscanf(f, "%d %s %c %d %d", &unused1, name_old, &state, &ppid, &pgid);

			node = (struct Process *)malloc(sizeof(struct Process));
			node->pid = unused1;
			node->pgid = pgid;
			node->name =find_name(name_old, 1, strlen(name_old) - 2);
			node->ppid = ppid;
			node->parent = NULL;
			node->child[0] = NULL;
			node->next = node_head;
			node_head = node;

			fclose(f);
		}
	}
}

struct Process *find_parent(int ppid)
{
	struct Process *node;
	for (node = node_head; node != NULL; node = node->next) {
		// printf("%d\n",node->pid);
		if (node->pid == ppid) {
			return node;
		}
	}
	return NULL;
}

void link_node()
{
	struct Process *pnode, *node;
	int i;
	for (node = node_head; node != NULL; node = node->next) {
		// printf("%d\n",node->pid);
		i = 0;
		pnode = find_parent(node->ppid);

		if (pnode != NULL) {
			node->parent = pnode;
			while (pnode->child[i++] != NULL);
			pnode->child[i - 1] = node;
			pnode->child[i] = NULL;
		}
	}
}

void find_root()
{
	struct Process *node;
	for (node = node_head; node != NULL; node = node->next) {
		if (node->parent == NULL) {
			node_root = node;
		}
	}
}

void version()//it is supposed to be one of the command, right?
{	
	printf("pstree (PSmisc) 22.21\n");
	printf("Copyright (C) 1993-2009 Werner Almesberger and Craig Small\n");
	printf("\n");
	printf("PSmisc comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it under\n");
	printf("the terms of the GNU General Public License.\n");
	printf("For more information about these matters, see the files named COPYING.\n");
}

char *find_name(char *str, int start, int end)
{
	int i;
	int size = (end - start) + 2;
	char *output = (char *)malloc(size * sizeof(char));
	for (i = 0; start <= end; start++, i++) {
		output[i] = str[start];
	}
	output[size] = '\0';
	return output;
}




int arr[300];
int arr_count = 0;

int length(int data)
{
	int n = 0;
	while (data) {
		n++;
		data /= 10;
	}
	return n;
}

void print_tree(struct Process *root, int level)
{
	int i;
	int k;
	struct Process *node;
	int count = 0;

	if (root->parent != NULL) {
		if (root->parent->child[0] != root) { 
			// not the first child

			for (i = 0; i < level; i++) {
				for (k = 0; k < arr_count; k++) {
					if (arr[k] == i) {
						printf("│");
					}
				}

				printf(" ");
			}

			printf("├─");

			count = count + 1;
		} else {
			if (root->parent->child[1] != NULL) { 
				// not only one child
				arr[arr_count++] = level;
				printf("┬");
				count = count + 2;
			} else {
				printf("─");
				count = count + 1;
			}
		}
	}
	printf("%s", root->name);
	if (root->child[0] == NULL){printf("\n");}

	//recursion on children
	int j = 0;
	while ((node = root->child[j++]) != NULL) {
		print_tree(node, level + strlen(root->name) + count);
	}
}

void print_tree_p(struct Process *root, int level)
{
	int i;
	int k;
	struct Process *node;
	int count = 0;
	if (root->parent != NULL) {
		if (root->parent->child[0] != root) { // not the first child

			for (i = 0; i < level; i++) {
				for (k = 0; k < arr_count; k++) {
					if (arr[k] == i) {
						printf("│");
					}
				}

				printf(" ");
			}

			printf("├─");

			count = count + 1;
		} else {
			if (root->parent->child[1] != NULL) { 
				// not only one child
				arr[arr_count++] = level;
				printf("┬");
				count = count + 2;
			} else {
				printf("─");
				count = count + 1;
			}
		}
	}
	int len = length(root->pid);

	printf("%s(%d)", root->name, root->pid);

	if (root->child[0] == NULL){printf("\n");}

	//recursion on children
	int j = 0;
	while ((node = root->child[j++]) != NULL) {
		print_tree_p(node, level + strlen(root->name) + count + len + 2);
	}
}

void print_tree_g(struct Process *root, int level)
{
	int i;
	int k;
	struct Process *node;
	int count = 0;
	if (root->parent != NULL) {
		if (root->parent->child[0] != root) { 
			// not the first child

			for (i = 0; i < level; i++) {
				for (k = 0; k < arr_count; k++) {
					if (arr[k] == i) {
						printf("│");
					}
				}

				printf(" ");
			}

			printf("├─");

			count = count + 1;
		} else {
			if (root->parent->child[1] != NULL) { 
				// not only one child
				arr[arr_count++] = level;
				printf("┬");
				count = count + 2;
			} else {
				printf("─");
				count = count + 1;
			}
		}
	}
	int len = length(root->pgid);

	printf("%s(%d)", root->name, root->pgid);

	if (root->child[0] == NULL){printf("\n");}

	//recursion on children
	int j = 0;
	while ((node = root->child[j++]) != NULL) {
		print_tree_g(node, level + strlen(root->name) + count + len + 2);
	}
}



int main(int argc, char *argv[])
{
	find_proc();
	link_node();
	find_root();

	if (argc == 1) {
		print_tree(node_root, 0);
	}

	else if (argc == 2) {
		if (!strcmp(argv[1], "-V")) {
			version();
		} else if (!strcmp(argv[1], "-p")) {
			print_tree_p(node_root, 0);
		} else if (!strcmp(argv[1], "-g")) {
			print_tree_g(node_root, 0);
		} else if (!strcmp(argv[1], "-c")) {
			print_tree(node_root, 0);
		} else if (!strcmp(argv[1], "-A")) {
			print_tree(node_root, 0);
		}
		else {
			printf("wrong option\nplease enter -V or -p or -g or -c or -A\n");
			return -1;
		}

	}

	else {
		printf("wrong input format\n");
	}

	return 0;
}