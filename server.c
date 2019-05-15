/**
*  Web Server - Starter Template
*
*  CSC 386-01 SP19 - Operating Systems Concepts
*  Programming Assignment #3
*  Missouri Western State University
*  Joseph Kendall-Morwick
*
*  Student Name:  Nhung Luu
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define SERVER_PORT 5567  // TODO: replace with your assigned port number
#define LOG_FILE "/var/csc386-sp19/qluu-prog3.log"
#define BUFFER_SIZE 4096
#define HTTP_SUCCESS_RESPONSE "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: Closed\n\n"
#define HTTP_404_RESPONSE "HTTP/1.1 404 Not Found\nContent-Type: text/html\nConnection: Closed\n\n"
#define WEBROOT "/var/www/html"
void *serve_get_request(void *);
char buffer[BUFFER_SIZE];

int main(int argc, char** argv) {
	//sockaddr_in address contains the internet address including the AF_INET(address - internet IP), port #, and IP address
	struct sockaddr_in address;
	int address_len = sizeof(address);
	//create a socket stating the type of connection family, socket type sock_stream is a TCP connection, and 0 is a protocol value
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	//if socket() return -1, then it it unable to create endpoint
	if(socket_fd == -1){
		printf("Unable to create endpoint through socket.");
		return 1;
	}	
	//define the values in the struct address
	address.sin_family = AF_INET;
	//inaddr_any will bound to all local address
	address.sin_addr.s_addr =  INADDR_ANY;
	//htons convert the host byte order to the required network byte order
	address.sin_port = htons(SERVER_PORT);
	//bind the socket to local interface address and port
	int bind_result = bind(socket_fd, (struct sockaddr *)&address, address_len);
	//if bind() return -1, then it is an error
	if(bind_result == -1){
		printf("Binding name to socket failed.");
		return 1;
	}	
	//listen for connection on the socket with 3 being the number of pending connection in queue allowed
	int listen_result = listen(socket_fd, 128);
	//check to see if the listen is working, 0 means it's working
	if(listen_result == 0){
		//use while loop to keep listening to server request
		while (listen_result == 0) {
			//accept a connection on a socket
			struct sockaddr_in client_socket;
			int client_fd = accept(socket_fd, (struct sockaddr *)&client_socket, &address_len);
			//check to see if able to accept connection, return -1 is error
			if(client_fd == -1){
				printf("Unable to accept connection.");
				return 1;
			}	
			//create a separate thread for every client, inlude the server get request function
			pthread_t pid;
			int thread_result = pthread_create(&pid, NULL, serve_get_request, (void*)client_fd);
			//check to see if thread is created successfully
			if(thread_result != 0){
				printf("Thread creation is unsuccessful.");
				return 1;
			}	
			int detach_result = pthread_detach(pid);
			if(detach_result != 0){
				printf("Unable to detach thread.");
				return 1;
			}	
		}
	}
	else{
		printf("Unable to successfully listen through socket");
		return 1;
	}
	return 0;
}

// TODO: implement the serve_get_request function.
void *serve_get_request(void* param) {
	int sock = (intptr_t)param;
	//open log file with append only
        FILE* logfp = fopen(LOG_FILE, "a");
        //check to see if the log file exist and leave a message on when the log file started
        if (logfp) {
                //include the time the log file is accessed
                time_t raw_time_accessed = time(NULL);
                struct tm* t = localtime(&raw_time_accessed);
                fprintf(logfp, "The server start at time %d %d, %d %d:%d:%d: ", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
	}
        //if log file does not exist, print error message
        else {
                fprintf(logfp, "The log file %s does not exist.", LOG_FILE);
        }
	// read HTTP GET request into a buffer
	int read_in = read(sock, buffer, BUFFER_SIZE);
	//check to see if able to read in file
	if(read_in == -1){
		fprintf(logfp, "Unable to read file through socket.");
		exit(1);
	}	
	// parse GET request
	char* token = strtok(buffer, " ");
	int strip_result = strcmp(token, "GET");
	if (strip_result != 0) { // we did not get a GET request
		// handle error
		fprintf(logfp, HTTP_404_RESPONSE);
		send(sock, HTTP_404_RESPONSE, strlen(HTTP_404_RESPONSE), MSG_NOSIGNAL);
	}
	//copy the path to another variable
	char* path = strtok(NULL, " ");
	char* file_path[BUFFER_SIZE];
	strcpy(file_path, WEBROOT);
	strcat(file_path, path);

	FILE* htmlfp = fopen(file_path, "r");
	//if the htmlfp file does not exist, send error message
	if (htmlfp) {
		send(sock, HTTP_SUCCESS_RESPONSE, strlen(HTTP_SUCCESS_RESPONSE), MSG_NOSIGNAL);
		int c = fgetc(htmlfp);
		while (c != EOF) {
			send(sock, &c, 1, 0);
			c = fgetc(htmlfp);
		}
		fprintf(logfp, "Hit: %s\n", path);
	}
	else {
		send(sock, HTTP_404_RESPONSE, strlen(HTTP_404_RESPONSE), MSG_NOSIGNAL);
		char* html = "Error: could not find file";
		send(sock, html, strlen(html), MSG_NOSIGNAL);
		fprintf(logfp, "Miss: %s\n", path);
	}

	//close file and socket connection
	fclose(logfp);
	close(sock);
}
