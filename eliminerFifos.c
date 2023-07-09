#include <unistd.h>

void eliminerFifos(char fifo1[], char fifo2[])
{
	int fd;
	fd = unlink(fifo1);
	if(fd != -1)
	{
		fd = unlink(fifo2);
		if(fd != -1)
		{
			
		}
		else
		{
			printf("Error when removing %s\n",fifo2);
		}
	}
	else
	{
		printf("Error when removing %s\n",fifo1);
	}
	
}
