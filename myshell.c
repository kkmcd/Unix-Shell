#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>

// Prototypes
char** parse();
void execute();
void prompt();
void check_redirection();

// Begin
int main() {
	char command[1024];
	char saved_command[1024];
	char **args;
	int i;

	printf("Starting Custom Shell...\n");

	// Main loop
	while(1) {

		// Display prompt
		prompt();

		// Get input from user
		fgets(command, 1024, stdin);

		// Save command for later use
		strcpy(saved_command, command);

		// Parse input
		args = parse(command);

		// Check for exit command
		if (strcmp(args[0], "exit") == 0) {
			_exit(0);
		}

		// Check for cd command
		else if (strcmp(args[0], "cd") == 0) {
			if (chdir(args[1]) == -1) {
				printf("Error: no directory found.\n");
			}
		}

		// Execute all other commands
		else {
			execute(args, saved_command);
		}
	}

	return(0);
}

// Display shell prompt
void prompt() {
	char pathname[1024];
	char *directory;

	// Determine current directory
	getcwd(pathname, sizeof(pathname));
	directory = basename(pathname);

	printf("%s", directory);
	printf(" $ ");
}

// Parse input
char** parse(char command[]) {
	const char delim[11] = " \t\n()<>|&;";
	char *token;
	char **tokens = malloc(128 * sizeof(char*));
	int index = 0;

	// Get first token
	token = strtok(command, delim);

	// Get remaining tokens
	while (token != NULL) {
		tokens[index] = token;
		index++;

		token = strtok(NULL, delim);
	}

	// Terminate argument array with NULL
	tokens[index] = NULL;

	return tokens;
}

// Execute
void execute(char *tokens[], char command[]) {
	int pid;

	pid = fork();

	// Child process
	if (pid == 0) {
		char *redirect;
		char *name;
		char file_name[1024];
		char *c;
		int i = 0;

		// Check for output redirection (append)
		if ((redirect = strstr(command, ">>"))) {
			
			// Move to next character
			c = redirect + (2*sizeof(char));

			name = strtok(c, " \n");

			strcpy(file_name, name);
			strcat(file_name, ".txt");

			// Find index of filename in token array
			while (strcmp(tokens[i], name) != 0) {
				i++;
			}
			
			// Remove filename from token array and shift following tokens to fill space
			while (tokens[i] != NULL) {
				tokens[i] = tokens[i+1];
				i++;
			}
			
			freopen(file_name, "a", stdout);
		}

		// Check for output redirection (new file)
		else if ((redirect = strstr(command, ">"))) {
			
			// Move to next character
			c = redirect + sizeof(char);

			name = strtok(c, " \n");

			strcpy(file_name, name);
			strcat(file_name, ".txt");

			// Find index of filename in token array
			while (strcmp(tokens[i], name) != 0) {
				i++;
			}

			// Remove filename from token array and shift following tokens to fill space
			while (tokens[i] != NULL) {
				tokens[i] = tokens[i+1];
				i++;
			}
			
			freopen(file_name, "w", stdout);
		}

		// Check for input redirection
		if ((redirect = strstr(command, "<")) != NULL) {

			// Move to next character
			c = redirect + sizeof(char);

			name = strtok(c, " \n");

			strcpy(file_name, name);
			strcat(file_name, ".txt");

			// Find index of filename in token array
			while (strcmp(tokens[i], name) != 0) {
				i++;
			}

			// Remove filename from token array and shift following tokens to fill space
			while (tokens[i] != NULL) {
				tokens[i] = tokens[i+1];
				i++;
			}
			
			freopen(file_name, "r", stdin);
		}

		// Execute command
		if (execvp(tokens[0], tokens) < 0) {
			// If execvp returns a value (-1) there has been an error
			printf("Error: execution failed\n");
		}

	}

	// Parent process
	else if (pid > 0) {
		int status;
		
		// Wait for child process to terminate
		wait(&status);
		
		// If status WIFEXITED returns false, there was a wait error
		if (!WIFEXITED(status)) {
			printf("Error: waiting failed\n");
			if (WIFSIGNALED(status)) {
				printf("Child terminated by signal");
			}
			else if (WCOREDUMP(status)) {
				printf("Child produced a core dump");
			}
		}
	}

	// If fork returns int < 0, fork failed
	else {
		printf("Error: fork failed\n");
	}
}