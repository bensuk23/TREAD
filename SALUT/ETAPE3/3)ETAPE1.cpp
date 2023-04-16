#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


void * fctThreadSLAVE(void *P);
void * fctThreadMASTER(void *P);
void fctThreadMASTERFin (void *param) ;
void handlerThreadSignal(int sig);
void handlerSIGUSR1(int sig);
pthread_t threadHandle1,threadHandle2,threadHandle3,threadHandle4,threadHandle5;

struct sigaction SIGAI;
struct sigaction SIGAU1;



int main()
{ 

	int * retThread1,* retThread2,* retThread3,* retThread4, ret;

	int paramEff = 5;

	sigemptyset(&SIGAI.sa_mask);
	SIGAI.sa_handler = handlerThreadSignal;
	sigaction(SIGINT, &SIGAI, NULL);

	sigemptyset(&SIGAU1.sa_mask);
	SIGAU1.sa_handler = handlerSIGUSR1;
	sigaction(SIGUSR1, &SIGAU1, NULL);
	sigset_t mask2;





	ret = pthread_create(&threadHandle5, NULL,(void*(*)(void*)) fctThreadMASTER,(void *) &paramEff);
	sleep(1);
	ret = pthread_create(&threadHandle1, NULL,(void*(*)(void*)) fctThreadSLAVE,(void *)  &paramEff);
	ret = pthread_create(&threadHandle2, NULL,(void*(*)(void*)) fctThreadSLAVE,(void *)  &paramEff);
	ret = pthread_create(&threadHandle3, NULL,(void*(*)(void*)) fctThreadSLAVE,(void *)  &paramEff);
	ret = pthread_create(&threadHandle4, NULL,(void*(*)(void*)) fctThreadSLAVE,(void *)  &paramEff);


	sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);
	sigaddset(&mask2, SIGINT);
	sigprocmask(SIG_SETMASK, &mask2, NULL);


	//pause();



	ret = pthread_join(threadHandle1, (void **)&retThread1);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread1);
	ret = pthread_join(threadHandle2, (void **)&retThread2);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread2);
	ret = pthread_join(threadHandle3, (void **)&retThread3);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread3);
	ret = pthread_join(threadHandle4, (void **)&retThread4);
	printf("Valeur renvoyee par le thread secondaire = %d\n", *retThread4);


	free(retThread1);
	free(retThread2);
	free(retThread3);
	free(retThread4);


	pthread_cancel(threadHandle5);


	puts("Fin du thread principal");
	pthread_exit(0);
}


void * fctThreadMASTER(void * P)
{
	sigset_t mask1;

	pthread_cleanup_push(fctThreadMASTERFin,NULL);

	sigemptyset(&mask1);
	sigaddset(&mask1, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask1, NULL);
	printf("je suis le thread => %u (MASTER) (FCTM)  \n",pthread_self());
	while(1)
	{
		pause();
	}

	pthread_cleanup_pop(1);

	pthread_exit(0);

}

void * fctThreadSLAVE(void * P)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	int * compteur = (int *)malloc(sizeof(int));
	*compteur = 0;
	
    printf("je suis le thread => %u (Attente d'un signal)   \n",pthread_self());
    pause();

	pthread_exit(compteur);
}



void handlerThreadSignal(int sig)
{
	printf("je suis le threads => %u (MASTER)   \n",pthread_self());
	printf("(Reception d'un signal)   \n");
	kill(getpid(),SIGUSR1);
}


void handlerSIGUSR1(int sig)
{
	printf("je suis le thread => %u (SLAVE)   \n",pthread_self());
	printf("(Reception d'un signalSIGUS1)   \n");

}

void fctThreadMASTERFin(void *param)
{
	printf("je suis thread => %u (MASTER) (FIN)   \n",pthread_self());
}