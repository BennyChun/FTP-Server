#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "utility.h"
#include "dir.h"
#include <sys/stat.h>

#define BACKLOG 5
#define USERNAME "CS317"
#define true 1
#define false 0

extern int newsoc_fd, newsoc_fd2, is_logged, newsoc_fd, quit_conn;
extern char root_wd[1024];
// extern char* actual_buffer;

// handle login
void handle_USER(char* buffer) {
  if (is_logged) {
		char* response = "503 Bad sequence of commands.\n";
    write(newsoc_fd, response, strlen(response));
  } else {
    char check_buffer[256];
    bzero(check_buffer, 256);
    strcpy(check_buffer, "USER ");
    strcat(check_buffer, USERNAME);
    if (strcasecmp("USER CS317\r\n", buffer) != 0) {
			// invalid
			char* response2 = "530 User not logged in.\n";
      write(newsoc_fd, response2, strlen(response2));
    } else {
      // valid
			is_logged = true;
			char* response3 = "230 User logged in, proceed.\n";
      write(newsoc_fd, response3, strlen(response3));
    }
  }
  memset(buffer,0,strlen(buffer));
  return;
}

// handle NLST
void handle_NLST(char* buffer) {

	// This is how to call the function in dir.c to get a listing of a directory.
	// It requires a file descriptor, so in your code you would pass in the file descriptor 
	// returned for the ftp server's data connection

	// write mark
	char* res = "150 File status okay; about to open data connection.\n";
	write(newsoc_fd, res, strlen(res));

	int buffer_size = strlen(buffer);
	// printf("the buffer length is : %d\n", strlen(buffer));
	if (strlen(buffer) > 6) {
		char* res1 = "501 Syntax error in parameters or arguments\n";
		write(newsoc_fd, res, strlen(res1));
	} else {
		// printf("Printed %d directory entries\n", listFiles(newsoc_fd2, "."));
	}

	// close data socket connection
	close(newsoc_fd2);
	char* final_res = "226 Closing data connection.\n";
	write(newsoc_fd, final_res, strlen(final_res));
	memset(buffer,0,strlen(buffer));
	return;
}

// handle CWD
void handle_CWD(char* buffer) {
	// ("i go here 1\n");
	// int buffer_size = strlen(actual_buffer);
	char* dir_query = substring(buffer, 4, strlen(buffer)-1);
	// printf("i go here 2\n");
	// printf("dir query is : %s\n", dir_query);
	if (strncmp(substring(dir_query, 0, 2), "./", 2) == 1 || strstr(dir_query, "../") != NULL || strstr(dir_query, "..") != NULL) {
		char* dir_resp = "504 Command not implemented for that parameter.\n";
		write(newsoc_fd, dir_resp, strlen(dir_resp));
	} else {
		// do cwd operation
		if (chdir(dir_query) != 0) {
			write(newsoc_fd, "550\n", 4);
			perror("error while changing dir");
		} else{
			write(newsoc_fd, "200\n", 4);			
		}
	}
	memset(buffer, 0, strlen(buffer));
	return;
}

// handle CDUP
void handle_CDUP(char* buffer) {
	char cwd[1024];

	/** THIS CODE MIGHT BREAK **/

	if (strlen(buffer) > 6) {
		// invalid number of parameters
		char* param_resp = "501 Syntax error in parameters or arguments.\n";
		write(newsoc_fd, param_resp, strlen(param_resp));
	} else {
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd() error");
		}
		if (!strncmp(root_wd, cwd, strlen(cwd))) {
			char* str = "503 Bad sequence of commands.\n";
			write(newsoc_fd, str, strlen(str));
		} else {
			if (chdir("../") != 0) {
				char* resp = "550 Requested action not taken. File unavailable, not found, not accessible.\n";
				write(newsoc_fd, resp, strlen(resp));
				perror("error while changing dir");
			} else{
				char* resp_thing = "200 Command okay.\n";
				write(newsoc_fd, resp_thing, strlen(resp_thing));			
			}
		}
	}
	memset(buffer, 0, strlen(buffer));
	return;
}

// handle QUIT
void handle_QUIT(buffer) {
	is_logged = false;
	quit_conn = true;
	char* quit_resp = "221 Service closing control connection.\n";
	write(newsoc_fd, quit_resp, strlen(quit_resp));
	memset(buffer, 0, strlen(buffer));
	return;
}

//NLST .
//CWD .