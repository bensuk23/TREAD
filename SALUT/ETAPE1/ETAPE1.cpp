#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void * fctThreadRECHMOT(int * param);
pthread_t threadHandle;



int main()
{ 
	int *retThread;
	int ret, paramEff = 5;

	ret = pthread_create(&threadHandle, NULL,(void*(*)(void*)) fctThreadRECHMOT, &paramEff);

	puts("Thread secondaire lance !");
	puts("Attente de la fin du thread secondaire");

	ret = pthread_join(threadHandle, (void **)&retThread);

	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread);

	free(retThread);

	puts("Fin du thread principal");
}

void * fctThreadRECHMOT (int * param)
{

	const char *cible = "print";

	int fd;
	int end = 0;
	int * compteur = (int *)malloc(sizeof(int));
	*compteur = 0;

	puts("th> Debut de thread");

	char buffer[50];

	printf("test");

	printf("Entrer une premiere chaine : ");



	for(int i=0;end == 0 ;i++)
	{
		printf("test\n");
		printf("test%d\n",*compteur);

		if((fd = open("testMOT", O_RDONLY)) == -1)
		{
			printf("fail\n");
			pthread_exit(compteur);
		}
		
		lseek(fd,i * sizeof(char),SEEK_SET);

		if((read(fd,buffer,strlen(cible))) < strlen(cible))
		{
			perror("Erreur de read(1)");
			end = -1;
		}
		printf("buf :%s\n",buffer);
		printf(" cible :%s\n",cible);
		if(strcmp(cible,buffer) == 0)
		{
			(*compteur)++;
		}

		close(fd);

	}

	pthread_exit(compteur);
}

