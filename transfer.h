/*
A header file that contains functions needed in the client-server transfer
 */

#include <stdio.h>

#define MSG_DIM 3 // size in bytes of the message to be sent
#define ATTEMPTS 3 // no of attempts to try sending / receiving a message
#define PACKAGE_SIZE 512 // size of the package

int sendProtocolMsg(int, void *, long);  // send a protocol message: size of message [in bytes] followed of the actual message
int recvProtocolMsg(int, void *);        // receive a protocol message: size of message [in bytes] followed of the actual message
int sendMsg(int, void *, long);          // tries sending the message for <ATTEMPTS> time
int recvMsg(int, void *, long);
int sendAll(int, void *, long *);        // sends the message specified
int recvAll(int, void *, long *);
int sendData(int, FILE *, long, long);         // sends data specified in the FILE * from the offset [a no of specific bytes]
long recvData(int, FILE *, long, long);

int createTree(char *, long, long, unsigned long); // fileName, fileSize, nrOfSegments
int getSegmentInfo(char *, long, long *, long *, long *); // fileName, segmentIndex, &offset, &segmentSize, &downloaded
long getSegmentNo (char *, unsigned long *);
int finalizeDownload(char *); //finalize the download by appending all the files.
void generateDirName(char *, char *);
void generateIndexName(char *, long, char *);
void generateSegmentName(char *, long, char *);
void generateSegmentNo(char *, char *);

unsigned long fileCrc(char *);
