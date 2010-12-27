/* 
 * My first stream server
 * Author: Karoly Szanto
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include  <sys/wait.h>
#include  <signal.h>

#define MYPORT 4444
#define BACKLOG 10

#define PORT 9034   // port we're listening on
#define MSG_DIM 3
#define ATTEMPTS 3
#define PACKAGE_SIZE 512

int sendProtocolMsg(int socket, void *message, int dim);
int recvProtocolMsg(int socket, void *message);
int sendMsg(int socket, void *message, long dim);
int recvMsg(int socket, void *message, long dim);
int sendAll(int socket, void *message, long *dim);
int recvAll(int socket, void *message, long *dim);
int sendMyData(int socket, FILE *file, long dim);

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
			
      char *message, *sendMsgString, *token;
      message = (char *)malloc(sizeof(char)*PACKAGE_SIZE);
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
      } while(ok);

      // send the file size
      fseek(myFile, 0, SEEK_END);
      long fileDim = ftell(myFile);
      sendMsgString = malloc((sizeof(long) + 5) * sizeof(char));
      sprintf(sendMsgString, "size %ld", fileDim);
      printf("Send msg: %s\n", sendMsgString);
      sendProtocolMsg(clientSocket, sendMsgString, strlen(sendMsgString));

      // wait for the start message
      recvProtocolMsg(clientSocket, message);
      printf("client message: %s\n", message);

      if(strcmp(message, "start") != 0) {
      	perror("Wrong client message ! [start was expected]");
      } else {
      	sendMyData(clientSocket, myFile, fileDim);
      }

      fclose(myFile);

      close(clientSocket);
      exit(0);
    }
		
    close(clientSocket);  // parent doesn't need this
  }
	
  return 0;
}



int sendProtocolMsg(int socket, void *message, int dim) {
  char msgDim[MSG_DIM+1];
  sprintf(msgDim,"%d",dim);
  if(sendMsg(socket, msgDim, MSG_DIM) == -1) {
    perror("send protocol message length");
    return -1;
  }

  if(sendMsg(socket, message, dim) == -1) {
    perror("send protocol message data");
    return -1;
  }

  return 0; 
}

int sendMsg(int socket, void *message, long dim) {
  long count = 0, sent = 0, d = dim;

  while(count < ATTEMPTS) {
    if(sendAll(socket, message + sent, &d) == -1) {
      sent += d;
      d = dim - sent;
    } else {
      return 0;
    }

    count++;
  }

  return -1;
}

int sendAll(int socket, void *message, long *dim) {
  long total = 0, ret;

  do {
    if((ret = send(socket, message + total, *dim - total, 0)) == -1) {
      break;
    } else {      
      total += ret;      
    }
  } while(total != *dim);

  *dim = total;

  return ret;
}

int sendMyData(int socket, FILE *file, long dim){
  fseek(file, 0, SEEK_SET);
  FILE* temp;
  if ((temp=fopen("temp.mpq","w"))==NULL) {
    perror("bleah");
  }
  long numBytes;
  void *buff = malloc(PACKAGE_SIZE);
  char *packageSize = malloc(sizeof(long) * sizeof(char));

  while((numBytes = fread(buff, 1, PACKAGE_SIZE, file)) > 0) {
    printf("Read from file: %d\n", numBytes);
    sprintf(packageSize, "%d", numBytes);
    fwrite(buff, 1, numBytes, temp);
    //sendMsg(socket, packageSize, strlen(packageSize));
    //sendMsg(socket, buff, numBytes);
    sendProtocolMsg(socket, buff, numBytes);
  }

  fclose(temp);
  return 0;
}

int recvProtocolMsg(int socket, void *message) {
  char *dimMsg = malloc(MSG_DIM + 1);
  printf("protocol message\n");
  if(recvMsg(socket, dimMsg, MSG_DIM) == -1) {
    perror("receive protocol message length");
    return -1;
  }

  dimMsg[MSG_DIM] = '\0';
  printf("message dim: %s\n", dimMsg);
  if(recvMsg(socket, message, atoi(dimMsg)) == -1) {
    perror("receive protocol message data");
    return -1;
  }

  ((char *)message)[atoi(dimMsg)] = '\0';
  printf("message as is: %s\n", message);
  return 0;
}

int recvMsg(int socket, void *message, long dim) {
  long count = 0, received = 0, d = dim;

  while(count < ATTEMPTS) {
    if(recvAll(socket, message + received, &d) == -1) {
      received += d;
      d = dim - received;
    } else {
      return 0;
    }

    count++;
  }

  return -1;
}

int recvAll(int socket, void *message, long *dim) {
  long total = 0, ret;
  
  do {
    if((ret = recv(socket, message + total, *dim - total, 0)) == -1) {
      break;
    } else {
      total += ret;
    }
  } while(total != *dim);

  *dim = total;

  return ret;
}
