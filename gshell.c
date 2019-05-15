/**
*  Griffon Shell - Starter Template
*
*  CSC 386-01 SP18 - Operating Systems Concepts
*  Programming Assignment #4
*  Missouri Western State University
*  Joseph Kendall-Morwick
*
*  Student Names: Nhung Luu
*/
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define BUFFER_SIZE 2048
#define MAX_COMMAND_LINE_ARGS 256
int main() {
	char input_line[BUFFER_SIZE];
	char* argv[MAX_COMMAND_LINE_ARGS];
	char path[BUFFER_SIZE];
	//read string from source to destination (path)
	strcpy(path, "/bin:/sbin:/usr/bin:/usr/sbin");
	printf("Welcome to GriffonShell!\n");
	while (1) { // infinite loop reading in commands from user
	// print command prompt
		printf("> ");
		// read a line of input from the user
		//stdin is user input
		//fgets() read input (stdin) into input_line and buffer_size is the maximum # of char to be read
		if (!fgets(input_line, BUFFER_SIZE, stdin)) {
			printf("error when reading input\n");
			return 1;
		}
		// remove trailing newline
		if (input_line[strlen(input_line) - 1] == '\n') input_line[strlen(input_line) - 1] = '\0'; // null termine 1 char earlier
			// parse command and arguments from input string
		int argc = 1;
		//strtok breaks input_line into string parts
		for (argv[0] = strtok(input_line, " "); argv[argc - 1]; argc++) {
			if (argc == MAX_COMMAND_LINE_ARGS) {
				printf("Too many arguments to command %s \n", argv[0]);
				return 2;
			}
			argv[argc] = strtok(NULL, " "); //read next token
		}
		argc--; // argc increments twice after last arg is found
		// check for built-in commands
		if (!argv[0] || !strlen(argv[0])) {  // no command entered
			continue;
		}
		else if (strcmp(argv[0], "exit") == 0) {  // exit command
			printf("Goodbye!\n");
			return 0;
		}
		else if (strcmp(argv[0], "setpath") == 0) {  // set path command
			if (argc != 2) {
				printf("Incorrect number of arguments: only a single colon-separated list of directory paths is expected\n");
			}
			else {
				strcpy(path, argv[1]);
				printf("new path: %s\n", path);
			}
		}
		//extra feature with the command cd
		else if(strcmp(argv[0], "cd") == 0){
			//this will change the current directory path to the one on argv[1] by adding the argv[1] to each pathway
			//new_dir will be the temporary individual path after it is broken up
			char* new_dir;
			//each_peath will the copied from new_dir to add on the cd cammand
			char each_path[BUFFER_SIZE];
			//tamp_path will be the temporary path as the new path is put together
			char temp_path[BUFFER_SIZE];
			//duplicate_path is a copy of the old path to be bloken up
			char duplicate_path[BUFFER_SIZE];
			//copy path to a duplicate path
			strcpy(duplicate_path, path);
			//while breaking up the duplicate path, modify it
			for(new_dir = strtok(duplicate_path, ":"); new_dir; new_dir = strtok(NULL, ":")){
				//copy the new dir to the temporary each_path
				strcpy(each_path, new_dir);
				//add the new cd command along with all the necessary char
				strcat(each_path, "/");
				strcat(each_path, argv[1]);
				strcat(each_path, ":");
				//if the tamp_path is empty, then just copy the first part of the path to tamp_path
				if(!temp_path || strlen(temp_path) == 0) strcpy(temp_path, each_path);
				//otherwise, add the next path
				else strcat(temp_path, each_path);
			}
			//overwrite the old path with the new path
			strcpy(path, temp_path);
			memset(temp_path, 0, sizeof(temp_path));
			//clear the tamp_path for future additional new cd command
			printf("new directory: %s\n", path);
		}	
		else { // attempt to execute program
			char* dir;
			int child_executed = 0;
			char path_copy[BUFFER_SIZE];
			char executable_path[BUFFER_SIZE];
			strcpy(path_copy, path);
			for (dir = strtok(path_copy, ":"); dir; dir = strtok(NULL, ":")) {
				//Start building the program to execute
				// TODO: create full path of executable (use strcat and strcpy -- check the man pages!)
				//copy the current dir to the executable_path
				strcpy(executable_path, dir);
				//adding the first argv to the executable_path
				strcat(executable_path, "/");
				strcat(executable_path, argv[0]);
				// TODO: check to see if the command file exists in dir and is executable
				if (executable_path && access(executable_path, X_OK) == 0) {
					/// TODO: fork off a process that executes the command and wait for it to finish
					//child ID
					pid_t pid;
					//fork a child process
					pid = fork();
					if (pid < 0) { 
						// error occured
						return 1;
					} 
					else if (pid == 0) { 
						//in child process
						//execute the path along with the argument
						execv(executable_path, argv);
					} 
					else { 
						// in parent process in which the parent is waiting for the child process to finish
						wait(NULL);
						//set child_executed = 1 to show that child process is complete
						child_executed = 1;
						if(child_executed == 1) printf("Child process is complete\n");
					}
				}
				else continue;
			}
			if (!child_executed) { // couldn't find the command
				printf("Command not found: %s\n", argv[0]);
			}
		}
	}
}
