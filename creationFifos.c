#include <sys/stat.h>

void creationFifos(char fifo1[], char fifo2[])
{
	int fd;
	fd = mkfifo(fifo1,0600);
	if( fd != -1)
	{
		fd = mkfifo(fifo2,0600);
		if(fd != -1)
		{
			printf("%s and %s created\n",fifo1,fifo2);
		}
		
	}
	
}
