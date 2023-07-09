#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "eliminerFifos.c"





#define MAX 30


void *lecture(void *arg)
{	
	char *fifo2 = (char *)arg;
	int fd2 = open(fifo2,O_RDONLY);
	char lec[512];

	do 
	{
		memset(lec,'\0',512);
		read(fd2, lec, 512);
		printf(">>%s\n", lec);

	} while(strcmp(lec, "/quitter"));
	close(fd2);
	return NULL;
}


int main(int argc, char *argv[])
{
	char fifo1[MAX];
	char fifo2[MAX]; 
	char user1[MAX];
	char user2[MAX];

	strcpy(fifo1, argv[1]);
	strcpy(fifo2, argv[2]);
	strcpy(user1, argv[3]);
	strcpy(user2, argv[4]);

	pthread_t pt;
	if(pthread_create(&pt, NULL, &lecture, &fifo2) == -1)
	{
		fprintf(stderr, "Error creating communication12 read pthread\n");
		exit(1);
	}

	int fd1 = open(fifo1,O_WRONLY);
	char entree[512];
	printf("Connection waiting with: %s\n",user2);
	printf("Conversation with %s\n",user2);

	do
	{
		memset(entree,'\0',512);
		fgets(entree,511,stdin);
		entree[strcspn(entree, "\n")] = 0;
		write(fd1,entree,512);
	} while(strcmp(entree,"/quitter"));
	close(fd1);

	eliminerFifos(fifo1,fifo2);
	exit(0);
}
