#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


struct Parametre
{
    int nb;
    char mots[10];
    char nomDuFichier[50];

}typedef PARAMETRE;

void * fctThreadRECHMOT(void *P);
pthread_t threadHandle1,threadHandle2,threadHandle3,threadHandle4;



int main()
{ 

	int * retThread1,* retThread2,* retThread3,* retThread4, ret;
    PARAMETRE P[5];
	int *retThread;



	for(int i=1;i<5;i++)
	{
		P[i].nb = i-1;
	    strcpy(P[i].mots,"print");
	}
	strcpy(P[1].nomDuFichier,"testMOT1");
	strcpy(P[2].nomDuFichier,"testMOT2");
	strcpy(P[3].nomDuFichier,"testMOT3");
	strcpy(P[4].nomDuFichier,"testMOT4");

	ret = pthread_create(&threadHandle1, NULL,(void*(*)(void*)) fctThreadRECHMOT,(void *)  &P[1]);
	ret = pthread_create(&threadHandle2, NULL,(void*(*)(void*)) fctThreadRECHMOT,(void *)  &P[2]);
	ret = pthread_create(&threadHandle3, NULL,(void*(*)(void*)) fctThreadRECHMOT,(void *)  &P[3]);
	ret = pthread_create(&threadHandle4, NULL,(void*(*)(void*)) fctThreadRECHMOT,(void *)  &P[4]);



	ret = pthread_join(threadHandle1, (void **)&retThread1);
	ret = pthread_join(threadHandle2, (void **)&retThread2);
	ret = pthread_join(threadHandle3, (void **)&retThread3);
	ret = pthread_join(threadHandle4, (void **)&retThread4);

	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread1);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread2);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread3);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread4);


	free(retThread1);
	free(retThread2);
	free(retThread3);
	free(retThread4);


	puts("Fin du thread principal");
}

void * fctThreadRECHMOT(void * P)
{

	int fd;
	int end = 0;
	int * compteur = (int *)malloc(sizeof(int));
	*compteur = 0;

	PARAMETRE * Pft;

    Pft = (PARAMETRE *)P;

	

	char buffer[50];

	for(int i=0;end == 0 ;i++)
	{


		if((fd = open(Pft->nomDuFichier,O_RDONLY)) == -1)
		{
			printf("fail\n");
			pthread_exit(compteur);
		}

		if(Pft->nb == 0)
        {
            printf("*\n");
        }
        else
        {
            if(Pft->nb == 1)
            {
                printf("\t*\n");
            }
            else
            {
                if(Pft->nb == 2)
                {
                    printf("\t\t*\n");
                }
                else
                {
                    if(Pft->nb == 3)
	                {
	                    printf("\t\t\t*\n");
	                }
            	}
            }
        }
		
		lseek(fd,i * sizeof(char),SEEK_SET);

		if((read(fd,buffer,strlen(Pft->mots))) < strlen(Pft->mots))
		{
			perror("Erreur de read(1)");
			end = -1;
		}
		if(strcmp(Pft->mots,buffer) == 0)
		{
			(*compteur)++;
		}

		close(fd);

	}

	pthread_exit(compteur);
}
