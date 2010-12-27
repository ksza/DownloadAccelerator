#define _POSIX_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include "transfer.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


/*********************** C R C *************************************/
/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James W. Williams of the University of Maryland.
 * Bug fix 1 June 1995 by ehg@research.att.com for 64-bit machines.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)crc.c	5.2 (Berkeley) 4/4/91";
#endif /* not lint */

#define _POSIX_SOURCE
#include <sys/types.h>
#include <unistd.h>

unsigned long crctab[] = {
  0x7fffffff,
  0x77073096,  0xee0e612c,  0x990951ba,  0x076dc419,  0x706af48f,
  0xe963a535,  0x9e6495a3,  0x0edb8832,  0x79dcb8a4,  0xe0d5e91e,
  0x97d2d988,  0x09b64c2b,  0x7eb17cbd,  0xe7b82d07,  0x90bf1d91,
  0x1db71064,  0x6ab020f2,  0xf3b97148,  0x84be41de,  0x1adad47d,
  0x6ddde4eb,  0xf4d4b551,  0x83d385c7,  0x136c9856,  0x646ba8c0,
  0xfd62f97a,  0x8a65c9ec,  0x14015c4f,  0x63066cd9,  0xfa0f3d63,
  0x8d080df5,  0x3b6e20c8,  0x4c69105e,  0xd56041e4,  0xa2677172,
  0x3c03e4d1,  0x4b04d447,  0xd20d85fd,  0xa50ab56b,  0x35b5a8fa,
  0x42b2986c,  0xdbbbc9d6,  0xacbcf940,  0x32d86ce3,  0x45df5c75,
  0xdcd60dcf,  0xabd13d59,  0x26d930ac,  0x51de003a,  0xc8d75180,
  0xbfd06116,  0x21b4f4b5,  0x56b3c423,  0xcfba9599,  0xb8bda50f,
  0x2802b89e,  0x5f058808,  0xc60cd9b2,  0xb10be924,  0x2f6f7c87,
  0x58684c11,  0xc1611dab,  0xb6662d3d,  0x76dc4190,  0x01db7106,
  0x98d220bc,  0xefd5102a,  0x71b18589,  0x06b6b51f,  0x9fbfe4a5,
  0xe8b8d433,  0x7807c9a2,  0x0f00f934,  0x9609a88e,  0xe10e9818,
  0x7f6a0dbb,  0x086d3d2d,  0x91646c97,  0xe6635c01,  0x6b6b51f4,
  0x1c6c6162,  0x856530d8,  0xf262004e,  0x6c0695ed,  0x1b01a57b,
  0x8208f4c1,  0xf50fc457,  0x65b0d9c6,  0x12b7e950,  0x8bbeb8ea,
  0xfcb9887c,  0x62dd1ddf,  0x15da2d49,  0x8cd37cf3,  0xfbd44c65,
  0x4db26158,  0x3ab551ce,  0xa3bc0074,  0xd4bb30e2,  0x4adfa541,
  0x3dd895d7,  0xa4d1c46d,  0xd3d6f4fb,  0x4369e96a,  0x346ed9fc,
  0xad678846,  0xda60b8d0,  0x44042d73,  0x33031de5,  0xaa0a4c5f,
  0xdd0d7cc9,  0x5005713c,  0x270241aa,  0xbe0b1010,  0xc90c2086,
  0x5768b525,  0x206f85b3,  0xb966d409,  0xce61e49f,  0x5edef90e,
  0x29d9c998,  0xb0d09822,  0xc7d7a8b4,  0x59b33d17,  0x2eb40d81,
  0xb7bd5c3b,  0xc0ba6cad,  0xedb88320,  0x9abfb3b6,  0x03b6e20c,
  0x74b1d29a,  0xead54739,  0x9dd277af,  0x04db2615,  0x73dc1683,
  0xe3630b12,  0x94643b84,  0x0d6d6a3e,  0x7a6a5aa8,  0xe40ecf0b,
  0x9309ff9d,  0x0a00ae27,  0x7d079eb1,  0xf00f9344,  0x8708a3d2,
  0x1e01f268,  0x6906c2fe,  0xf762575d,  0x806567cb,  0x196c3671,
  0x6e6b06e7,  0xfed41b76,  0x89d32be0,  0x10da7a5a,  0x67dd4acc,
  0xf9b9df6f,  0x8ebeeff9,  0x17b7be43,  0x60b08ed5,  0xd6d6a3e8,
  0xa1d1937e,  0x38d8c2c4,  0x4fdff252,  0xd1bb67f1,  0xa6bc5767,
  0x3fb506dd,  0x48b2364b,  0xd80d2bda,  0xaf0a1b4c,  0x36034af6,
  0x41047a60,  0xdf60efc3,  0xa867df55,  0x316e8eef,  0x4669be79,
  0xcb61b38c,  0xbc66831a,  0x256fd2a0,  0x5268e236,  0xcc0c7795,
  0xbb0b4703,  0x220216b9,  0x5505262f,  0xc5ba3bbe,  0xb2bd0b28,
  0x2bb45a92,  0x5cb36a04,  0xc2d7ffa7,  0xb5d0cf31,  0x2cd99e8b,
  0x5bdeae1d,  0x9b64c2b0,  0xec63f226,  0x756aa39c,  0x026d930a,
  0x9c0906a9,  0xeb0e363f,  0x72076785,  0x05005713,  0x95bf4a82,
  0xe2b87a14,  0x7bb12bae,  0x0cb61b38,  0x92d28e9b,  0xe5d5be0d,
  0x7cdcefb7,  0x0bdbdf21,  0x86d3d2d4,  0xf1d4e242,  0x68ddb3f8,
  0x1fda836e,  0x81be16cd,  0xf6b9265b,  0x6fb077e1,  0x18b74777,
  0x88085ae6,  0xff0f6a70,  0x66063bca,  0x11010b5c,  0x8f659eff,
  0xf862ae69,  0x616bffd3,  0x166ccf45,  0xa00ae278,  0xd70dd2ee,
  0x4e048354,  0x3903b3c2,  0xa7672661,  0xd06016f7,  0x4969474d,
  0x3e6e77db,  0xaed16a4a,  0xd9d65adc,  0x40df0b66,  0x37d83bf0,
  0xa9bcae53,  0xdebb9ec5,  0x47b2cf7f,  0x30b5ffe9,  0xbdbdf21c,
  0xcabac28a,  0x53b39330,  0x24b4a3a6,  0xbad03605,  0xcdd70693,
  0x54de5729,  0x23d967bf,  0xb3667a2e,  0xc4614ab8,  0x5d681b02,
  0x2a6f2b94,  0xb40bbe37,  0xc30c8ea1,  0x5a05df1b,  0x2d02ef8d
};

/*
 * crc --
 *	Compute a POSIX.2 checksum.  This routine has been broken out since
 *	it is anticipated that other programs will use it.  It takes a file
 *	descriptor to read from and locations to store the crc and the number
 *	of bytes read.  It returns 0 on success and 1 on failure.  Errno is
 *	set on failure.
 */
int crc(int fd, unsigned long *cval, unsigned long *clen) {
  register int i, nr, step;
  register unsigned char *p;
  register unsigned long crcv, total;
  unsigned char buf[8192];

  crcv = step = total = 0;
  while ((nr = read(fd, buf, sizeof(buf))) > 0)
    for (total += nr, p = buf; nr--; ++p) {
      if (!(i = crcv >> 24 ^ *p)) {
	i = step++;
	if (step >= sizeof(crctab)/sizeof(crctab[0]))
	  step = 0;
      }
      crcv = ((crcv << 8) ^ crctab[i]) & 0xffffffff;
    }
  if (nr < 0)
    return(1);

  *cval = crcv;
  *clen = total;
  return(0);
}

               
unsigned long fileCrc(char *file){
  int fd;
  unsigned long cval, clen;   
   
  if((fd = open(file,O_RDONLY))<0){   
    perror("unable to check crc for file");
    return -1;
  }else{
    crc(fd,&cval,&clen);         
    close(fd);
    return cval;
  }
}
/********************** E N D  O F  C R C *************************/

int sendProtocolMsg(int socket, void *message, long dim) {
  char msgDim[MSG_DIM + 1];
  // creates the string that contains the size of the next message
  sprintf(msgDim,"%ld", dim);
  if(sendMsg(socket, msgDim, MSG_DIM) == -1) {
    perror("send protocol message length");
    return -1;
  }

  //sends the actual message
  if(sendMsg(socket, message, dim) == -1) {
    perror("send protocol message data");
    return -1;
  }

  return 0; 
}

int sendMsg(int socket, void *message, long dim) {
  long count = 0, sent = 0, d = dim;
  // tries to send the message for <ATTEMPTS> times
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

// sends data from the specified file.
int sendData(int socket, FILE *file, long offset, long segmentSize){
  fseek(file, offset, SEEK_SET);
  long numBytes, total = 0;
  char buff[PACKAGE_SIZE];
  FILE *ftemp = fopen("tmp.txt", "w");
  //while the numBytes != 0 [we didn't hit the end of file], sends the data through the socket
  while((numBytes = fread(buff, 1, PACKAGE_SIZE, file)) > 0) {
    total += numBytes;
    if(total  <= segmentSize) {
      fwrite(buff, 1, numBytes, ftemp);
      sendProtocolMsg(socket, buff, numBytes);
    } else {
      fwrite(buff, 1, segmentSize - (total - numBytes), ftemp);
      sendProtocolMsg(socket, buff, segmentSize - (total - numBytes));
      break;
    }
   
    printf("Sent: %ld\n", numBytes);
  }

  fclose(ftemp);
  return 0;
}

// receive a protocol message
int recvProtocolMsg(int socket, void *message) {
  char dimMsg[MSG_DIM + 1];
  //printf("in recvProtocolMsg\n");
  // read the size of the following message
  if(recvMsg(socket, dimMsg, MSG_DIM) == -1) {
    perror("receive protocol message length");
    return -1;
  }

  // read the actual message
  dimMsg[MSG_DIM] = '\0';
  //printf("in recvProtocolMsg dimMsg: %s\n", dimMsg);
  if(recvMsg(socket, message, atoi(dimMsg)) == -1) {
    perror("receive protocol message data");
    return -1;
  }

  //printf("in recvProtocolMsg message: %s\n", (char *) message);

  ((char *)message)[atoi(dimMsg)] = '\0';
  // return the actual number of bytes read
  return atoi(dimMsg);
}


int recvMsg(int socket, void *message, long dim) {
  long count = 0, received = 0, d = dim;

  while(count < ATTEMPTS) {
    if(recvAll(socket, message + received, &d) == -1) {
      received += d;
      d = dim - received;
      count++;
      //printf("received %ld\n",received);
    } else {
      return received==0?dim:received;
    }
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
      //printf("received in recvAll: %ld",ret);
    }
  } while(total != *dim);

  *dim = total;
  return ret;
}


// receive data over the network
// offset -> in the temp file; dataSize -> how much is left to download
long recvData(int socket, FILE* file, long offset, long dataSize) {
  long received = 0, total = 0;
  char buffer[PACKAGE_SIZE + 1];  

  //fseek(file, offset, SEEK_SET);  
  while (total < dataSize) {
    if ((received = recvProtocolMsg(socket, buffer)) < 0) {
      perror("Error during transfer");
      return -1;
    }
   
    fwrite(buffer, 1, received, file);
    fflush(file);
    total += received;
    //printf("Received: %ld, total: %ld\n", received, total); 
  }
  return total;
}

// nrOfSegments -> max 100
int createTree(char *fileName, long fileSize, long nrOfSegments, unsigned long originalCRC) {
  int i;
  long segmentSize = fileSize / nrOfSegments, offset = 0;
  // ./fileName.dir/fileName.i.tmp\0
  char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];
  FILE *tmp;
  generateDirName(fileName, tempFile);
  if(mkdir(tempFile, 0777)) {
    int errsv = errno;
    perror("error creating directory !");
    if (errsv == EEXIST) {
      perror("look at the @h)le?");
      return 1;
    }
    else return -1;
   
  }
  for(i = 1; i <= nrOfSegments; i++) {
    if(i == nrOfSegments) segmentSize += fileSize % nrOfSegments;

    generateIndexName(fileName, i, tempFile);
    if((tmp = fopen(tempFile, "w")) == NULL) {
      perror("error creating index file");
      return -1;
    }
    fprintf(tmp, "%ld %ld\n", offset, segmentSize);
    fclose(tmp);

    offset += segmentSize;
   
    generateSegmentName(fileName, i, tempFile);
    if((tmp = fopen(tempFile, "w")) == NULL) {
      perror("error creating segment file");
      return -1;
    }
    //man fprintf(tmp,"%d\n",i);
    fclose(tmp);
  }
  generateSegmentNo(fileName, tempFile);
  if((tmp = fopen(tempFile, "w")) == NULL) {
    perror("error creating segment number file");
    return -1;
  }
  fprintf(tmp, "%ld\n", nrOfSegments);
  fprintf(tmp, "%lu\n", originalCRC);
  fclose(tmp);

  return 0;
}

int getSegmentInfo(char *fileName, long segmentIndex, long *offset, long *segmentSize, long *downloaded) {
  FILE *file;
  char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];
  generateIndexName(fileName,segmentIndex, tempFile);

  if((file = fopen(tempFile, "r")) == NULL) {
    perror("error opening index file");
    return -1;
  }
  fscanf(file, "%ld %ld", offset, segmentSize);
  fclose(file);

  generateSegmentName(fileName, segmentIndex, tempFile);
  if((file = fopen(tempFile, "r")) == NULL) {
    perror("error opening index file");
    return -1;
  }
  fseek(file, 0, SEEK_END);
  *downloaded = ftell(file);
  fclose(file);

  return 0;
}

long getSegmentNo(char *fileName, unsigned long *CRC) {
  FILE* file;
  long segmentNo;
  char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];
  generateSegmentNo(fileName, tempFile);
  if((file = fopen(tempFile, "r")) == NULL) {
    perror("error opening index file");
    return -1;
  }
  fscanf(file, "%ld", &segmentNo);
  fscanf(file, "%lu", CRC);
  fclose(file);
  return segmentNo;
}


int finalizeDownload(char *fileName) {
  unsigned long originalCRC, finalCRC;
  long segmentNo = getSegmentNo(fileName, &originalCRC);
  long i, offset, segmentSize, downloaded;
  FILE* final, *temp;
  char tempFile[2 * strlen(fileName) + 4 + 3 + 5 + 4];

  if((final = fopen(fileName, "w")) == NULL) {
    perror("error creating downloaded file");
    return -1;
  }
   
  for(i=1;i<=segmentNo;i++) {
    printf("saving segment %ld\n", i);
    int numBytes;
    char buff[PACKAGE_SIZE];
    if ( getSegmentInfo(fileName,i,&offset, &segmentSize, &downloaded) < 0 ) {
      perror("error retrieving segment info");
      return -1;
    }
    /*
      if (ftell(final)!=offset){
      perror("error finalizing file. File corrupted");
      return -1;
      }
    */
    generateSegmentName(fileName, i, tempFile);
    if((temp = fopen(tempFile, "r")) == NULL) {
      perror("error opening segment file");
      return -1;
    }

    while((numBytes = fread(buff, 1, PACKAGE_SIZE, temp)) > 0){
      fwrite(buff, 1, numBytes, final);
      fflush(final);
    }
    fclose(temp);
   
    if (unlink(tempFile)==-1){
      perror("Could not delete temporary file. Please remove manually");
    }
    generateIndexName(fileName, i, tempFile);
    if (unlink(tempFile)==-1){
      perror("Could not delete index file. Please remove manually");
    }
  }
  generateSegmentNo(fileName, tempFile);
  if (unlink(tempFile)==-1){
    perror("Could not delete segment number file. Please remove manually");
  }
  generateDirName(fileName, tempFile);
  rmdir(tempFile);
  finalCRC = fileCrc(fileName);
  printf("%lu ?! %lu", finalCRC, originalCRC);
  if (finalCRC != originalCRC) {
      perror("Downloaded CRC does not match with original file's CRC");      
  }
  fclose(final);
  printf("saved final file\n");
  return 0;
}

void generateDirName(char *fileName, char *dirName ){
  sprintf(dirName, "%s%s%s", "./", fileName, ".dir/");
}

void generateIndexName(char *fileName, long index, char *indexName) {
  char dirName[2 + strlen(fileName) + 6];
  generateDirName(fileName,dirName);
  sprintf(indexName, "%s%s%s%ld", dirName, fileName, ".", index);
}

void generateSegmentName(char *fileName, long index, char *segmentName) {
  char dirName[2 + strlen(fileName) + 6];
  generateDirName(fileName,dirName);
  sprintf(segmentName, "%s%s%s%ld%s", dirName, fileName, ".", index, ".tmp");
}


void generateSegmentNo(char *fileName, char *segmentNo) {
  char dirName[2 + strlen(fileName) + 6];
  generateDirName(fileName,dirName);
  sprintf(segmentNo, "%s%s%s", dirName, fileName, ".no");
}
