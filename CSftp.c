#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include "dir.h"
#include "usage.h"
#include "utility.h"
#include "accesscmd.h"
#include "pasv.h"

typedef int bool;

#define BACKLOG 5
#define USERNAME "CS317"
#define true 1
#define false 0

int port_num, serv_fd, newsoc_fd, addr_len, cli_len;

bool is_logged = false, quit_conn = false, is_binary = false;

struct sockaddr_in serv_addr, cli_addr;
char buffer[256], root_wd[1024], actual_buffer[256];

// parse client response
void parse_response(char* buffer) {
  memset(actual_buffer, 0, strlen(actual_buffer));
  strcpy(actual_buffer, buffer);
  // printf("i in actual buffer %s\n", actual_buffer);
  int i = 0;
  while (buffer[i] != '\0') {
    buffer[i] = toupper(buffer[i]);
    i++;
  }

  // printf("This is the buffer %s and it's size is %d\n", buffer, strlen(buffer));
  // strcpy(buffer, temp_buffer);
  if (strncmp("USER", buffer, 4) == 0) {
    handle_USER(buffer);
  } else {
    if (is_logged) {
      if (strncmp("QUIT", buffer, 4) == 0) {
        handle_QUIT(buffer);
      } else if (strncmp("CWD", buffer, 3) == 0) {
        handle_CWD(actual_buffer);
        memset(buffer,0,strlen(buffer));
      } else if (strncmp("CDUP", buffer, 4) == 0) {
        handle_CDUP(buffer);
      } else if (strncmp("TYPE", buffer, 4) == 0) {
        handle_TYPE(buffer);
      } else if (strncmp("MODE", buffer, 4) == 0) {
        handle_MODE(buffer);
      } else if (strncmp("STRU", buffer, 4) == 0) {
        handle_STRU(buffer);
      } else if (strncmp("RETR", buffer, 4) == 0) {
        handle_RETR(actual_buffer);
        memset(buffer,0,strlen(buffer));
      } else if (strncmp("PASV", buffer, 4) == 0) {
        handle_PASV(buffer);
      } else if (strncmp("NLST", buffer, 4) == 0) {
        handle_NLST(buffer);
      } else {
        char* response = "500 Syntax error, command unrecognized, command line too long.\n";
        write(newsoc_fd, response, strlen(response));
      }
    } else {
      char* repsonse1 = "530 User not logged in.\n";
      write(newsoc_fd, repsonse1, strlen(repsonse1));
      memset(buffer,0,strlen(buffer));
    }
  }
  return;
}

int main(int argc, char **argv) {
    
    // Check the command line arguments
    if (argc != 2) {
      usage(argv[0]);
      return -1;
    }
    
    // get port number
    port_num = atoi(argv[1]);
    
    // create socket
    if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      perror("error creating socket");
      exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(serv_addr);

    // bind socket to the specified port
    if (bind(serv_fd, (struct sockaddr *) &serv_addr, addr_len) < 0) {
      perror("error while binding");
      exit(EXIT_FAILURE);
    }

    // listen for clients
    if (listen(serv_fd, BACKLOG) < 0) {
      perror("listen fail");
      exit(EXIT_FAILURE);
    }

    cli_len = sizeof(cli_addr);

    while (1) {

      // accept connection
      if ((newsoc_fd = accept(serv_fd, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_len)) < 0) {
        perror("error while accepting");
        exit(EXIT_FAILURE);
      } else {
        quit_conn = 0;
      }

      // send 220 signal
      char* response2 = "220 Service ready for new user.\n";
      write(newsoc_fd, response2, strlen(response2));

      //bzero(buffer, 256);
      memset(buffer,0,strlen(buffer));

      // store the servers root directory -- move it outside the while loop
      if (getcwd(root_wd, sizeof(root_wd)) == NULL) {
        perror("getcwd() error");
      }
      // printf("the root dir is : %s\n", root_wd);
    
      if (read(newsoc_fd, buffer, 255) < 0) {
        perror("error reading from socket");
        exit(EXIT_FAILURE);
      }

      // handle the client response
      parse_response(buffer);
      do {
        if (read(newsoc_fd, buffer, 255) < 0) {
          perror("error reading from socket");
          exit(EXIT_FAILURE);
        }
        parse_response(buffer);
        memset(buffer,0,strlen(buffer));
      } while (quit_conn == false);
      close(newsoc_fd);
      
    }

    return 0;

}
