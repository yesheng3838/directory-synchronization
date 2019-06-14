#include "copia.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int copia(int fdo, int fdd)
{
	struct stat buf;
	size_t mapSize;
	char* textoO;
	char* textoD;
	int i;

	if(fstat(fdo,&buf) == -1) {
		perror("Fallo en fstat");
		close(fdo);
		close(fdd);
		exit(-1);
	}

	if(ftruncate(fdd, buf.st_size)) {
		perror("Fallo en truncate destino");
	    exit(1);
	}

	if (!S_ISREG(buf.st_mode))
	{
	   	printf("No es un archivo regular ni un enlace simbolico, no se copia\n");
		close(fdo);
		close(fdd);
		exit(-1);
	}

	mapSize = buf.st_size;

	if ((textoO = mmap(0, mapSize, PROT_READ | PROT_WRITE,MAP_SHARED,fdo, (off_t)0)) == NULL) {
	 	perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	if ((textoD = mmap(0, mapSize, PROT_READ | PROT_WRITE,MAP_SHARED,fdd, (off_t)0)) == NULL) {
	 	perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < mapSize; i++){
		textoD[i] = textoO[i];
	}

  	munmap(textoO, mapSize);
	munmap(textoD, mapSize);

	return (int)mapSize;
}

