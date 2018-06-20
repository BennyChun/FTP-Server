#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pasv.h"
#include "utility.h"
#include <netdb.h>

typedef int bool;
#define true 1
#define false 0
#define CHUNK 4096
#define BACKLOG 5

int serv_fd2, addr_len2, newsoc_fd2, port_num2, cli_len;
struct sockaddr_in serv_addr2;

extern int newsoc_fd;
extern bool is_binary;
extern struct sockaddr_in cli_addr;

void handle_PASV() {

	char pasv_response[256];
	char hostname[1024];
	struct hostent *he; 
	struct in_addr **addr_list;
	char* ip_str;
	int ip[4];
	struct timeval timeout;

	/* Wait up to twenty seconds. */
	timeout.tv_sec = 20;
	timeout.tv_usec = 0;

	// create a new socket 
	if ((serv_fd2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("error creating socket");
		exit(EXIT_FAILURE);
	}

	// init sockaddr_in and bind
	serv_addr2.sin_family = AF_INET;
	serv_addr2.sin_port = 0;
	serv_addr2.sin_addr.s_addr = INADDR_ANY;
	
	addr_len2 = sizeof(serv_addr2);

	if (setsockopt (newsoc_fd2, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	if (setsockopt (newsoc_fd2, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	// bind socket to the specified port
	if (bind(serv_fd2, (struct sockaddr *) &serv_addr2, addr_len2) < 0) {
		perror("error while binding");
		exit(EXIT_FAILURE);
	}

	if (getsockname(serv_fd2, (struct sockaddr*) &serv_addr2, (socklen_t*) &addr_len2) < 0) {
		perror("getsockname failed");
		exit(EXIT_FAILURE);
	}

	// get hostname
	gethostname(hostname, sizeof hostname);

	// get ip
	if ((he = gethostbyname(hostname)) == NULL) {
		herror("gethostbyname");
		return;
	}

	addr_list = (struct in_addr **)he->h_addr_list;
	ip_str = inet_ntoa(*addr_list[0]);
	sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	int port_num3 = ntohs(serv_addr2.sin_port);
	// print information about this host:
	// printf("Local port is: %d\n", port_num3);
	// printf("i will print the ip is %s\n", ip_str);

	// listen for client
	if (listen(serv_fd2, BACKLOG) < 0) {
		perror("listen fail");
		exit(EXIT_FAILURE);
	}

	// structure pasv response
	char port1_str[256];
	char port2_str[256];
	int port1 = port_num3/256;
	int port2 = port_num3%256;
	
	itoa(port1, port1_str, 10);
	itoa(port2, port2_str, 10);

	sprintf(pasv_response, "227 Entering Passive Mode (%d, %d, %d, %d, %d, %d)\n", ip[0], ip[1], ip[2], ip[3], port1, port2);

	// write response
	write(newsoc_fd, pasv_response, strlen(pasv_response));

	// accept connection
	if ((newsoc_fd2 = accept(serv_fd2, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_len)) < 0) {
		perror("error while accepting");
		exit(EXIT_FAILURE);
	}

	return;
}

void handle_TYPE(char* buffer) {
	int buffer_size = strlen(buffer);
	char* type = substring(buffer, 5, buffer_size - 1);
	// printf("the tpye is %s\n", type);
	if (!strcasecmp(type, "A")) {
		is_binary = false;
	} else if (!strcasecmp(type, "I")) {
		is_binary = true;
		// printf("the val of binary flag is %d\n", is_binary);
	} else {
		// invalid params
		write(newsoc_fd, "504\n", 4);
		return;
	}
	write(newsoc_fd, "200\n", 4);
	return;
}

void handle_MODE(char* buffer) {
	int buffer_size = strlen(buffer);
	char* mode = substring(buffer, 5, buffer_size - 1);
	if (strcasecmp(mode, "S") != 0) {
		// reject
		if ((strcasecmp(mode, "B") == 0) || (strcasecmp (mode, "C") == 0)) {
			write(newsoc_fd, "504\n", 4);
		} else {
			write(newsoc_fd, "501\n", 4);
		}
	} else {
		// accept
		write(newsoc_fd, "200\n", 4);
	}
	return;
}

void handle_STRU(char* buffer) {
	int buffer_size = strlen(buffer);
	char* stru = substring(buffer, 5, buffer_size - 1);
	if (strcasecmp(stru, "F") != 0) {
		// reject
		write(newsoc_fd, "504\n", 4);
	} else {
		// accept
		write(newsoc_fd, "200\n", 4);
	}
	return;
}

// handle RETR command
void handle_RETR(char* buffer) {
	char file_buf[CHUNK];	

	// get file name 

	/** CHANGE IN LUNIX EVERYTIME */

	char* filename = substring(buffer, 5, strlen(buffer) - 1);

	/** CHANGE ENDS **/

	// printf("the file name is %s end of buffer\n", filename);

	FILE *fp;
	size_t nread;
	if (!is_binary) {
		// retr text file

		// open a file
		if (!(fp = fopen(filename, "r"))) {
			char* response10 = "550 Requested action not taken. File unavailable, not found, not accessible.\n";
			write(newsoc_fd, response10, strlen(response10));
			perror("file doesnt exist");
			return;
		} else {

			// send mark
			write(newsoc_fd, "150\n", 4);

			// write file to data socket
			while((nread = fread(file_buf, sizeof file_buf, 1, fp)) > 0) {
				fwrite(file_buf, 1, nread, stdout);
			}

			if (ferror(fp)) {
				/** INSERT ERROR CODE **/
				write(newsoc_fd, "552\n", 4);
			}

			// write chunk to data socket
			write(newsoc_fd2, file_buf, 4096);

			fclose(fp);
			
			// successfully transferred 
			close(newsoc_fd2);
			char* re1 = "226 Closing data connection.\n";
			write(newsoc_fd, re1, strlen(re1));
		}

	} else {
		// retr binary files

		// open a file in binary mode

		if (!(fp = fopen(filename, "rb"))) {
			char* ree = "550 Requested action not taken. File unavailable, not found, not accessible.\n";
			write(newsoc_fd, ree, strlen(ree));
			perror("file doesnt exist");
			return;
		} else {

			// send mark
			char* response = "150 File status okay; about to open data connection.\n";
			write(newsoc_fd, response, strlen(response));

			// write file to data socket
			while((nread = fread(file_buf, sizeof file_buf, 1, fp)) > 0) {
				fwrite(file_buf, 1, nread, stdout);
			}

			fseek(fp, 0, SEEK_END);
			long filelen = ftell(fp);
			char* ret = malloc(filelen);
			fseek(fp, 0, SEEK_SET);
			fread(ret, 1, filelen, fp);
			if (ferror(fp)) {
				char* response = "500 Syntax error, command unrecognized.\n";
				write(fp, response, strlen(response));
			}
			fclose(fp);
			write(newsoc_fd2, ret, filelen);
			close(newsoc_fd2);
			char* response_success = "226 Closing data connection, file transfer successful\n";
			write(newsoc_fd, response_success, strlen(response_success));
			
			// successfully transferred 
			close(newsoc_fd2);
			char* re = "226 Closing data connection.\n";
			write(newsoc_fd, re, strlen(re));
		}


	}
	memset(buffer,0,strlen(buffer));
	return;
}
