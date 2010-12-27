/* 
 * My first stream server
 * Author: Karoly Szanto
 */

#include "transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>

#define MYPORT 4445
#define BACKLOG 10

void sigchld_handler(int s) {
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(void) {
  int serverSocket, clientSocket; // listen on server socket, new clonnection with client on clientSocket
  struct sockaddr_in serverAddr; // informations regarding the server's address
  struct sockaddr_in clientAddr; // informations regarding the client's address
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;         // for setsockopt() SO_REUSEADDR, below


  if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // lose the pesky "address already in use" error message
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
	
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(MYPORT);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
	
  if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof serverAddr) == -1) {
    perror("bind");
    exit(1);
  }
	
  if(listen(serverSocket, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
	
  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
	
  while(1) { // main accept() loop
    sin_size = sizeof clientSocket;
		
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientSocket, &sin_size)) == -1) {
      perror("accept");
      continue;
    }
		
    printf("server: got connection from %s\n", inet_ntoa(clientAddr.sin_addr));
    if (! fork()) { // this is the child process
      close(serverSocket); // child doesn't need the listener
			
      char message[PACKAGE_SIZE + 1], *token;
      
      // recieve first message
      int ok = 1;
      char *fileName;
      FILE *myFile;
      do {
	recvProtocolMsg(clientSocket, message);			
	printf("client message: %s\n", message);
	token = strtok(message, " ");
	token = strtok(NULL, " ");

	fileName = malloc(strlen(token) + 2);
	sprintf(fileName, "%s%s", ".", token);

	if((myFile = fopen(fileName, "r")) == NULL) {
	  sendProtocolMsg(clientSocket, "Error: no such file !", strlen("Error: no such file !"));
	} else {
	  sendProtocolMsg(clientSocket, "Success: file does exist !", strlen("Succes: file does exist !"));
	  ok= 0;
	}

	free(fileName);
      } while(ok);

      // send the file size
      fseek(myFile, 0, SEEK_END);
      long fileDim = ftell(myFile);
      unsigned long fCrc = fileCrc(fileName);
      char sendMsgString[sizeof(long) + 6];
      sprintf(sendMsgString, "size %ld %lu", fileDim, fCrc);      
      printf("Send msg: %s\n", sendMsgString);
      sendProtocolMsg(clientSocket, sendMsgString, strlen(sendMsgString));

      // wait for the start message
      recvProtocolMsg(clientSocket, message);
      printf("client message: %s\n", message);

      token = strtok(message, " ");
      if(strcmp(token, "start") != 0) {
      	perror("Wrong client message ! [start was expected]");
      } else {
	long offset, segmentSize;
	token = strtok(NULL, " ");
	offset = atol(token);
	token = strtok(NULL, " ");
	segmentSize = atol(token);
      	sendData(clientSocket, myFile, offset, segmentSize);
      }

      fclose(myFile);

      close(clientSocket); 
      printf("server:  client %s disconnected\n", inet_ntoa(clientAddr.sin_addr));
      
      exit(0);
    }
		
    close(clientSocket);  // parent doesn't need this
  }
	
  return 0;
}
