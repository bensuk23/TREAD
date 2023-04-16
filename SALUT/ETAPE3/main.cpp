#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>





void * fctThread(void *P);
void handlerSIGINT(int sig);
void destructeur(void *p);

/*void handlerThreadSignal(int sig);*/

pthread_t threadHandle[4];

/*struct sigaction SIGAI;
struct sigaction SIGAU1;*/

int Compteur;


typedef struct
{
char nom[20];
int nbSecondes;
} DONNEE;

DONNEE data[] = { "MATAGNE",15,
				  "WILVERS",10,
				  "WAGNER",17,
				  "QUETTIER",8,
				  "",0 };

DONNEE Param;
pthread_mutex_t mutexDATA;
pthread_mutex_t mutexComp;
pthread_cond_t condComp;

pthread_key_t cle;

struct sigaction SIGAI;


int main()
{ 

	
	int ret;

	
	int i=0;

	sigemptyset(&SIGAI.sa_mask);
	SIGAI.sa_handler = handlerSIGINT;
	sigaction(SIGINT, &SIGAI, NULL);


	pthread_mutex_init(&mutexDATA, NULL);
	pthread_mutex_init(&mutexComp, NULL);
	pthread_cond_init(&condComp, NULL);

	pthread_key_create(&cle,destructeur);

	sigset_t mask2;
	sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);
	sigaddset(&mask2, SIGINT);
	sigprocmask(SIG_SETMASK, &mask2, NULL);


	while(data[i].nbSecondes != 0)
	{

		pthread_mutex_lock(&mutexDATA);
		pthread_mutex_lock(&mutexComp);
		memcpy(&Param,&data[i],sizeof(DONNEE));


		ret = pthread_create(&threadHandle[i], NULL,(void*(*)(void*)) fctThread,(void *) &Param);
		printf("salut\n");


		i++;
	}
	
	

	pthread_mutex_lock(&mutexComp);
	while(Compteur)
	{
		printf("sddsdfgfef%d \n",Compteur);
		pthread_cond_wait(&condComp, &mutexComp);
	}
	pthread_mutex_unlock(&mutexComp);



	/*sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);
	sigaddset(&mask2, SIGINT);
	sigprocmask(SIG_SETMASK, &mask2, NULL);*/


	//pause();


	//pthread_cancel(threadHandle5);


	puts("Fin du thread principal");
	pthread_exit(0);
}


void * fctThread(void * P)
{
	//sigset_t mask1;
	char *NOMrecup;

	sigset_t mask2;
	sigemptyset(&mask2);
	sigprocmask(SIG_SETMASK, &mask2, NULL);

	Compteur++;
	pthread_mutex_unlock(&mutexComp);

	printf("%d \n",Compteur);
	
	DONNEE * Pft;

    Pft = (DONNEE *)P;

    struct timespec temps;
    struct timespec temps2;

    NOMrecup = (char *)malloc(100);

    

    strcpy(NOMrecup,Pft->nom);
    printf("---> nom = %s", NOMrecup);

	temps.tv_sec = Pft->nbSecondes;
	temps.tv_nsec = 0;

	pthread_mutex_unlock(&mutexDATA);
    
    if (pthread_setspecific(cle,NOMrecup))
	{
		puts("!!!! Erreur de setspecific");
	}




	/*sigemptyset(&mask1);
	sigaddset(&mask1, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask1, NULL);*/
	printf("je suis le thread => %d.%u et je m'occupe de : %s   \n",getpid(),pthread_self(),NOMrecup);

	printf("test\n");
	while (nanosleep(&temps, &temps) != 0);
	
	printf("je suis le thread => %d.%u et je me termine :\n",getpid(),pthread_self());

	pthread_mutex_lock(&mutexComp);
	Compteur--;
	pthread_mutex_unlock(&mutexComp);
	//printf("%d",Compteur);
	pthread_cond_signal(&condComp);

	//pthread_cleanup_pop(1);
	return 0;



}


void handlerSIGINT(int sig)
{

	char * NOM;
	//NOM = (char *)malloc(100);
	NOM =  (char*)pthread_getspecific(cle);

	printf("(Reception d'un signal SIGINT)   \n");
	printf("je suis le threads => %u , je m'occupe de : %s   \n",pthread_self(),NOM);
	

}

void destructeur (void *p)
{
	puts("=== liberation d'une zone specifique ===");
	free(p);
}

