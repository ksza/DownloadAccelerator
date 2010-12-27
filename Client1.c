/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include "transfer.h"
#include <sys/types.h>
#include <sys/wait.h>

#define MAXDATASIZE 100 // max number of bytes we can get at once
#define PORT 4445   // port we're listening on
int main(int argc, char *argv[]) {
	int sockfd;
	char buf[PACKAGE_SIZE+1];
	char* servers[10];
	struct hostent *he;
	struct sockaddr_in their_addr; // connector's address information 

	if (argc != 5) {
		fprintf(stderr,"usage: Servers_list File_requested Received_file_path Nr_of_segments\n");
		exit(1);
	}

	//citire servere
	int i= 0, nrServers;
	FILE *serv;
	if ((serv = fopen(argv[1], "r")) == NULL) {
		perror("error opening segment file");
		return -1;
	}
	while (!feof(serv)) {
		char server[15];
		fscanf(serv, "%15s", server);
		servers[i]=malloc(strlen(server)+1);
		sprintf(servers[i], "%s", server);
		printf("server: %s\n", servers[i]);
		i++;
	}
	nrServers=i;
	int iterator=0;
	for (iterator=0; iterator<nrServers; iterator++) {

		if ((he=gethostbyname(servers[iterator])) == NULL) { // get the host info 
			herror("gethostbyname");
			exit(1);
		}

		if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}

		their_addr.sin_family = AF_INET; // host byte order 
		their_addr.sin_port = htons(PORT); // short, network byte order 
		their_addr.sin_addr = *((struct in_addr *)he->h_addr);
		memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

		if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof their_addr)
				== -1) {
			perror("connect");
		}
		
		printf("connected to server: %s\n",servers[iterator]);

		char requestMsg[(strlen(argv[2])+5)];
		sprintf(requestMsg, "ask %s", argv[2]);
		printf("%s\n", requestMsg);
		sendProtocolMsg(sockfd, requestMsg, strlen(requestMsg));
		if (recvProtocolMsg(sockfd, buf)<0) {
			perror("Error receiving");
		} else
			break;
	}
	printf("%s\n", buf);

	//expected the size of the file
	if (recvProtocolMsg(sockfd, buf)<0) {
		perror("Error receiving");
	}
	printf("%s\n", buf);
	long filesize;
	long unsigned originalCRC;
	char *token=strtok(buf, " ");
	token=strtok(NULL," ");
	if (strncmp(buf, "size", 4)==0) {
		filesize=atol(token);
		printf("%ld\n", filesize);
	}

	token=strtok(NULL," ");
	originalCRC= strtoul(token, NULL, 10);
	printf("crc: %lu\n", originalCRC);

	int nrOfSegments = atoi(argv[4]);
	if (nrOfSegments>filesize/MAXDATASIZE) {
		nrOfSegments = filesize/MAXDATASIZE < 100 ? (int)(filesize/MAXDATASIZE)
				+1 : 99;
	}

	printf("Nr of segments: %d", nrOfSegments);

	char fileName [strlen(argv[3])+1];
	sprintf(fileName, "%s", argv[3]);
	int check;
	if ((check=createTree(fileName, filesize, nrOfSegments, originalCRC))<0) {
		exit(-1);
	}

	//we already have a started download with this name
	if (check==1) {
		unsigned long readCRC;
		char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];
		generateSegmentNo(fileName, tempFile);
		FILE *noFile;
		if ((noFile = fopen(tempFile, "r")) == NULL) {
			perror("error opening number of segments file");
			return -1;
		}
		fscanf(noFile, "%d", &nrOfSegments);
		fscanf(noFile, "%lu", &readCRC);
		if (readCRC != originalCRC) {
			perror("downloaded file does not match requested file");
			return -1;
		}
		//printf("new number of segments ;) %d \n " , nr
		fclose(noFile);
	}

	i= 1;
	int j=0;
	int status;
	for (; i<=nrOfSegments; i++) {
		int pid;
		if ((pid=fork())<0) {
			perror("Error forking!");
			exit(3);
		}
		if (pid==0) {

			printf("child nr %d\n", i);
			int childSockfd;

			struct hostent *he1;
			struct sockaddr_in their_addr1; // connector's address information 

			for (j=iterator; j<nrServers; j++) {

				if ((he1=gethostbyname(servers[j])) == NULL) { // get the host info 
					herror("gethostbyname");
					exit(1);
				}

				if ((childSockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
					perror("socket");
					exit(1);
				}

				their_addr1.sin_family = AF_INET; // host byte order 
				their_addr1.sin_port = htons(PORT); // short, network byte order 
				their_addr1.sin_addr = *((struct in_addr *)he1->h_addr);
				memset(their_addr1.sin_zero, '\0', sizeof their_addr1.sin_zero);

				if (connect(childSockfd, (struct sockaddr *)&their_addr1,
						sizeof their_addr1) == -1) {
					perror("connect");
				} else
					break;
			}

			printf("child %d %d\n", i, childSockfd);

			char requestMsg[(strlen(argv[2])+5)];
			sprintf(requestMsg, "ask %s", argv[2]);
			printf("%s\n", requestMsg);
			sendProtocolMsg(childSockfd, requestMsg, strlen(requestMsg));
			if (recvProtocolMsg(childSockfd, buf)<0) {
				perror("Error receiving");
			}
			printf("child %d %s\n %d\n", i, buf, childSockfd);

			//expected the size of the file
			// this is just so that the protocol will work
			if (recvProtocolMsg(childSockfd, buf)<0) {
				perror("Error receiving");
			}
			//printf("%s\n", buf);
			long filesize;
			char *token=strtok(buf, " ");
			token=strtok(NULL," ");
			if (strncmp(buf, "size", 4)==0) {
				filesize=atol(token);
				printf("%ld\n", filesize);
			}

			// ./fileName.dir/fileName.i.tmp\0
			char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];
			FILE *tmp;
			generateIndexName(fileName, i, tempFile);
			if ((tmp = fopen(tempFile, "r")) == NULL) {
				perror("error opening index file");
				return -1;
			}
			// citire din fisier index
			long segmentSize, offset;
			fscanf(tmp, "%ld %ld", &offset, &segmentSize);
			fclose(tmp);

			generateSegmentName(fileName, i, tempFile);

			int readBytes=0;
			if (check == 1) {
				if ((tmp = fopen(tempFile, "a")) == NULL) {
					perror("error opening segment file");
					return -1;
				}
				readBytes = ftell(tmp);
				fclose(tmp);
			}

			//long = 32 => 2*long + "start  \0" = 72
			char startMsg[72];
			offset+=readBytes;
			segmentSize-=readBytes;
			sprintf(startMsg, "start %ld %ld", offset, segmentSize);

			// initiating download
			if (sendProtocolMsg(childSockfd, startMsg, strlen(startMsg))<0) {
				perror("Error sending \"start\" download");
				exit(3);
			}

			if ((tmp = fopen(tempFile, "a")) == NULL) {
				perror("error opening segment file");
				return -1;
			}

			if (recvData(childSockfd, tmp, offset, segmentSize) < 0) {
				perror("Error saving to file");
				exit(4);
			}
			printf("saved to file\n");
			fflush(tmp);
			fclose(tmp);
			printf("file closed\n");

			close(childSockfd);
			exit(0);
		}

	}

	for (i=0; i<nrOfSegments; i++)
		wait(&status);
	printf("done\n");
	finalizeDownload(fileName);
	close(sockfd);
	return 0;
}
