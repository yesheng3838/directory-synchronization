#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

static int (*copia)(int, int);

struct dirent * buscar(struct dirent *origen, char *nomDestino, int *correcto)
{
	DIR *dirDestino;
	struct dirent *des = NULL;

	*correcto = 0;
	if((dirDestino = opendir(nomDestino)) == NULL)
	{
		perror("Error en funcion buscar:");
		exit(1);
	}

	des = readdir(dirDestino);
	while((des != NULL) && (strcmp(des->d_name, origen->d_name) != 0))
		des = readdir(dirDestino);

	if(des == NULL)
		*correcto = -1;

	closedir(dirDestino);

	return des;
}

int sincro(DIR *dirOrigen, char *nomDestino, char *nomOrigen)
{
	DIR *dirDestino;
	struct dirent *origen, *destino = NULL;
	int fdOrigen, fdDestino;
	struct stat infoOrigen, infoDestino;
	int busqueda = 0;
	char rutaArchivoOrigen[4096], rutaArchivoDestino[4096];
	int err = 0;

	while ((origen = readdir(dirOrigen)) != NULL)
	{
		strcpy(rutaArchivoOrigen, nomOrigen);
		strcat(rutaArchivoOrigen, "/");
		strcat(rutaArchivoOrigen, origen->d_name);

		if (lstat(rutaArchivoOrigen, &infoOrigen) == -1)
		{
			perror("stat sincro origen");
			exit(1);
		}

		destino = buscar(origen, nomDestino, &busqueda);
		if((S_ISREG(infoOrigen.st_mode)) && (busqueda == -1)) // No esta el fichero en destino
		{
			strcpy(rutaArchivoDestino, nomDestino);
			strcat(rutaArchivoDestino, "/");
			strcat(rutaArchivoDestino, origen->d_name);

			if((fdDestino = open(rutaArchivoDestino, O_CREAT | O_RDWR | O_TRUNC, (infoOrigen.st_mode))) == -1)
			{
			  perror("open archivo destino en archivo no existente: ");
			  exit(1);
			}
			if((fdOrigen = open(rutaArchivoOrigen, O_RDWR)) == -1)
			{
			  perror("open archivo origen en archivo no existente: ");
			  exit(1);
			}

			(*copia)(fdOrigen, fdDestino);
			close(fdDestino);
			close(fdOrigen);
			if((chown(rutaArchivoDestino, infoOrigen.st_uid, infoOrigen.st_gid)) == -1)
			{
				perror("chown archivo no existente: ");
				exit(1);
			}
		}
		else if((S_ISLNK(infoOrigen.st_mode)) && (busqueda == -1)) // Enlace Simbolico
		{
			char buffer[1024];
			int error = 0;
			struct stat simbolico;

			strcpy(rutaArchivoDestino, nomDestino);
			strcat(rutaArchivoDestino, "/");
			strcat(rutaArchivoDestino, origen->d_name);

			lstat(rutaArchivoOrigen, &simbolico);
			if ((error = readlink(rutaArchivoOrigen, buffer, simbolico.st_size)) == -1)
			{
				perror("readlink: ");
				exit(1);
			}
			if ((error = symlink(buffer,rutaArchivoDestino)) == -1)
			{
				perror("symlink: ");
				exit(1);
			}
			chmod(origen->d_name, infoOrigen.st_mode);
		}
		else if((S_ISREG(infoOrigen.st_mode)) && (busqueda == 0) && (strcmp(origen->d_name, destino->d_name) == 0))// El archivo existe
		{
			strcpy(rutaArchivoDestino, nomDestino);
			strcat(rutaArchivoDestino, "/");
			strcat(rutaArchivoDestino, destino->d_name);

			if (stat(rutaArchivoDestino, &infoDestino) == -1)
			{
				perror("stat sincro destino: ");
				exit(1);
			}

			if((infoDestino.st_mtim.tv_sec - infoOrigen.st_mtim.tv_sec) != 0)
			{
				if((fdDestino = open(rutaArchivoDestino, O_RDWR | O_TRUNC)) == -1)
				{
				  perror("open archivo destino modificacion: ");
				  exit(1);
				}
				if((fdOrigen = open(rutaArchivoOrigen, O_RDWR)) == -1)
				{
				  perror("open archivo origen modificacion: ");
				  exit(1);
				}
				(*copia)(fdOrigen, fdDestino);
				close(fdDestino);
				close(fdOrigen);
			}

			if(infoOrigen.st_uid != infoDestino.st_uid) // No tiene el mismo propietario
			{
				if((err = chown(rutaArchivoDestino, infoOrigen.st_uid, infoOrigen.st_gid)) == -1)
				{
					perror("chown archivo existente: ");
					exit(1);
				}
			}

			if(infoOrigen.st_mode != infoDestino.st_mode) //No tiene los mismos permisos
			{
				if((err = chmod(rutaArchivoDestino, infoOrigen.st_mode)) == -1)
				{
					perror("chmod archivo existente: ");
					exit(1);
				}
			}
		}

		// Reseteo de cadenas
		memset(rutaArchivoOrigen,'\0',strlen(rutaArchivoOrigen));
		memset(rutaArchivoDestino,'\0',strlen(rutaArchivoDestino));
	}
	closedir(dirOrigen);

	if((dirDestino = opendir(nomDestino)) == NULL)
	{
	  perror("open DirDestino sincro: ");
	  exit(1);
	}

	// Recorrer el dirDestino para eliminar los archivos que no esten en dirOrigen
	struct dirent *archivoDestino;
	while((archivoDestino = readdir(dirDestino)) != NULL)
	{
		destino = buscar(archivoDestino, nomOrigen, &busqueda);

		if(busqueda == -1)
		{
			strcpy(rutaArchivoDestino, nomDestino);
			strcat(rutaArchivoDestino, "/");
			strcat(rutaArchivoDestino, archivoDestino->d_name);

			if((err = unlink(rutaArchivoDestino)) == -1)
			{
				perror("unlink borrar archivos no existentes en origen: ");
				exit(1);
			}
			memset(rutaArchivoDestino,'\0',strlen(rutaArchivoDestino));
		}
	}

	closedir(dirDestino);

	return 0;
}

int main(int argc, char *argv[])
{
	  FILE * fp;
	  char *origen = NULL, *destino = NULL;
	  DIR   *dirOrigen, *dirDestino;
	  size_t lenO = 0, lenD = 0, lenB = 0;
	  ssize_t aux;
	  struct stat infoDIR;

	  // Biblioteca Dinamica
	  char *biblioteca = NULL;
	  void *handle;
	  char *error;
	  //////////////

	  umask(0000);
	/* Apertura del fichero solo lectura */
	  fp = fopen("<Path of sincro.conf>","r");

	/* Comprobacion de errores */
	  if (fp == NULL)
		  exit(EXIT_FAILURE);

	  // Lectura del directorio origen
	  if((aux = getline(&origen, &lenO, fp)) == -1)
		  perror("readOrigen sincro.conf: ");
	  
	   // Lectura del directorio destino
	  if((aux = getline(&destino, &lenD, fp)) == -1)
		  perror("readDestino sincro.conf: ");

	  // Lectura de la ruta de la biblioteca
	  if((aux = getline(&biblioteca, &lenB, fp)) == -1)
		  perror("readbiblioteca sincro.conf: ");

		
	  origen[strlen(origen)-1] = '\0';
	  destino[strlen(destino)-1] = '\0';
	  biblioteca[strlen(biblioteca)-1] = '\0';

	  fclose(fp);

	  handle = dlopen (biblioteca, RTLD_LAZY);
	  if (!handle)
	  {
			fprintf (stderr, "%s\n", dlerror());
			exit(1);
	  }

	  copia = dlsym(handle, "copia");
	  if ((error = dlerror()) != NULL)
	  {
		fprintf (stderr, "%s\n", error);
		exit(1);
	  }


	  if((dirOrigen = opendir(origen)) == NULL)
	  {
	  	  perror("open sincro.conf: ");
	  	  exit(1);
	  }

	  if((dirDestino = opendir(destino)) == NULL)
	  {
		  if (stat(origen, &infoDIR) == -1)
		  {
			  perror("stat sincro origen");
			  exit(1);
		  }

		  if(mkdir(destino, infoDIR.st_mode) == -1)
		  {
		  	  perror("Error en mkdir de dirDestino: ");
		  	  exit(1);
		  }

		  if((chown(destino, infoDIR.st_uid, infoDIR.st_gid)) == -1)
		  {
			perror("chown creacion directorio destino: ");
			exit(1);
		  }
	  }

	  closedir(dirDestino);
	  sincro(dirOrigen, destino, origen);

	  dlclose(handle);

	  if (origen)
		 free(origen);

	  if (destino)
		 free(destino);

	return 0;
}

