#include "copia.h"
#include <unistd.h>
#include <stdio.h>

int copia(int fdo, int fdd)
{
	int bytes = 0;
	char buffer[1024];
	int numbytes;

	while ((numbytes = read(fdo, &buffer, sizeof(char))) > 0)
	{
	    if((write(fdd, &buffer, numbytes)) == -1)
	    {
	    	perror("Error en copia:");
	    	return -1;
	    }
	    else
	    	bytes += numbytes;
	}


	return bytes;
}
