#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);


void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

void afficherS();

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;
pthread_t threadCroco;
pthread_t threadCorbeau;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;
pthread_mutex_t mutexdelaiEnnemis;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = false;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;
int VIE_DKJR = 0;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

// ------------------------------------------------------------------------
struct sigaction I;
struct sigaction AL;
struct sigaction IN;
struct sigaction U1;
struct sigaction U2;
struct sigaction CH;
struct sigaction HP;

pthread_once_t controleur = PTHREAD_ONCE_INIT;


int main(int argc, char* argv[])
{
	int evt;
	int ret1,ret2,ret3,ret4;

	pthread_mutex_init(&mutexGrilleJeu, NULL);
	pthread_mutex_init(&mutexDK, NULL);
	pthread_mutex_init(&mutexEvenement, NULL);
	pthread_mutex_init(&mutexScore, NULL);
	pthread_mutex_init(&mutexdelaiEnnemis, NULL);

	pthread_cond_init(&condDK, NULL);
	pthread_cond_init(&condScore, NULL);

	pthread_key_create(&keySpec, DestructeurVS);
	sigset_t mask1;

	ouvrirFenetreGraphique();

	sigemptyset(&I.sa_mask);
	I.sa_handler = HandlerSIGQUIT;
	sigaction(SIGQUIT, &I, NULL);

	sigemptyset(&IN.sa_mask);
	IN.sa_handler = HandlerSIGINT;
	sigaction(SIGINT, &IN, NULL);

	sigemptyset(&CH.sa_mask);
	CH.sa_handler = HandlerSIGCHLD;
	sigaction(SIGCHLD, &CH, NULL);

	sigemptyset(&HP.sa_mask);
	HP.sa_handler = HandlerSIGHUP;
	sigaction(SIGHUP, &HP, NULL);

	sigemptyset(&AL.sa_mask);
	AL.sa_handler = HandlerSIGALRM;
	sigaction(SIGALRM, &AL, NULL);
	
	sigemptyset(&U1.sa_mask);
	U1.sa_handler = HandlerSIGUSR1;
	sigaction(SIGUSR1, &U1, NULL);

	sigemptyset(&U2.sa_mask);
	U2.sa_handler = HandlerSIGUSR2;
	sigaction(SIGUSR2, &U2, NULL);


	

	sigemptyset(&mask1);
	sigaddset(&mask1,SIGQUIT);
	sigaddset(&mask1,SIGINT);
	sigaddset(&mask1,SIGCHLD);
	sigaddset(&mask1,SIGHUP);
	sigaddset(&mask1,SIGALRM);
	sigaddset(&mask1,SIGUSR2);
	sigaddset(&mask1,SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask1, NULL);


	ret1 = pthread_create(&threadCle, NULL,(void*(*)(void*)) FctThreadCle,(void *) NULL);
	ret2 = pthread_create(&threadDKJr, NULL,(void*(*)(void*)) FctThreadDKJr,(void *) NULL);
	ret2 = pthread_create(&threadDK, NULL,(void*(*)(void*)) FctThreadDK,(void *) NULL);
	ret2 = pthread_create(&threadScore, NULL,(void*(*)(void*)) FctThreadScore,(void *) NULL);
	ret2 = pthread_create(&threadEnnemis, NULL,(void*(*)(void*)) FctThreadEnnemis,(void *) NULL);
	ret3 = pthread_create(&threadEvenements, NULL,(void*(*)(void*)) FctThreadEvenements,(void *) NULL);

	while(VIE_DKJR < 3)
	{
		ret4 = pthread_join(threadDKJr, NULL);
		VIE_DKJR ++;
		afficherEchec(VIE_DKJR);
		if(VIE_DKJR < 3)
		{
			ret2 = pthread_create(&threadDKJr, NULL,(void*(*)(void*)) FctThreadDKJr,(void *) NULL);
		}	
	}


	sleep(5);
	
	

	exit(0);
	


}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}



void* FctThreadCle(void *)
{
	int i=1;
	int test=0; 

	struct timespec temps;

	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;
	while(1)
  {
  	pthread_mutex_lock(&mutexGrilleJeu);
  	printf("i = %d\n",i);

  	afficherCle(i);
  	if(i==1)
  	{
  		setGrilleJeu(0,1,4,pthread_self());
  	}
  	else if(i>=2)
		{
			setGrilleJeu(0,1,0,pthread_self());
		}
		//afficherGrilleJeu();
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps, NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
		effacerCarres(3, 12, 2, 3);
  	if(test==0)
  	{
  		i++;
  		if(i==5)
	  	{
	  		test=1;
	  		i=4;
	  	}
  	}
  	if(test==1)
  	{
  		i--;
  		if(i==0)
	  	{
	  		test=0;
	  		i=2;
	  	}
  	}
  	pthread_mutex_unlock(&mutexGrilleJeu);
  }
}

void* FctThreadEvenements(void *)
{

	struct timespec temps;

	temps.tv_sec = 0;
	temps.tv_nsec = 100000000;

	int evt;

	while (1)
	{
		pthread_mutex_lock(&mutexEvenement);
    evt= lireEvenement();

    printf("EVT : %d\n",evt);

	    
   if(evt == SDL_QUIT)
				exit(0);
	evenement = evt;
	pthread_mutex_unlock(&mutexEvenement);
	kill(getpid(),SIGQUIT);

	 nanosleep(&temps, NULL);
	 evenement = AUCUN_EVENEMENT;
	}
}



void* FctThreadDKJr(void* p)
{
	bool on = true;
	sigset_t mask;

	struct timespec tempsjump;
	struct timespec tempsanim;

	tempsjump.tv_sec = 1;
	tempsjump.tv_nsec = 400000000;
	tempsanim.tv_sec = 0;
	tempsanim.tv_nsec = 500000000;

	


	sigfillset(&mask);
	sigdelset(&mask, SIGQUIT);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGCHLD);
	sigdelset(&mask, SIGHUP);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	pthread_mutex_lock(&mutexGrilleJeu);

	if(grilleJeu[3][1].tid !=0)
		pthread_kill(grilleJeu[3][1].tid,SIGUSR2);
	if(grilleJeu[3][2].tid !=0)
		pthread_kill(grilleJeu[3][2].tid,SIGUSR2);
	if(grilleJeu[3][3].tid !=0)
		pthread_kill(grilleJeu[3][3].tid,SIGUSR2);
	if(grilleJeu[2][0].tid !=0)
		pthread_kill(grilleJeu[2][0].tid,SIGUSR1);
	if(grilleJeu[2][1].tid !=0)
		pthread_kill(grilleJeu[2][1].tid,SIGUSR1);
	if(grilleJeu[2][2].tid !=0)
		pthread_kill(grilleJeu[2][2].tid,SIGUSR1);

	setGrilleJeu(3, 1, DKJR,pthread_self());
	afficherDKJr(11, 9, 1);

	etatDKJr = LIBRE_BAS;

	positionDKJr = 1;
	afficherGrilleJeu();

	pthread_mutex_unlock(&mutexGrilleJeu);





	while(on)
	{
		pause();
		pthread_mutex_lock(&mutexEvenement);

		pthread_mutex_lock(&mutexGrilleJeu);

		printf(" ETAT DK : %d\n",etatDKJr);

		switch (etatDKJr)
		{
				case LIBRE_BAS:
					switch (evenement)
					{
							case SDLK_LEFT:

							if (positionDKJr > 1)
							{
								setGrilleJeu(3, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr--;
								if(grilleJeu[3][positionDKJr].type == CROCO)
								{
									kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_exit(0);
								}
								setGrilleJeu(3, positionDKJr, DKJR,pthread_self());
								afficherGrilleJeu();
								afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
								etatDKJr = LIBRE_BAS;
							}
							break;

							case SDLK_RIGHT:
							if (positionDKJr < 7)
							{
								setGrilleJeu(3, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr++;
								if(grilleJeu[3][positionDKJr].type == CROCO)
								{
									kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_exit(0);
								}
								setGrilleJeu(3, positionDKJr, DKJR,pthread_self());
								afficherGrilleJeu();
								afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							}
							
							break;
							case SDLK_UP:

							if(grilleJeu[2][positionDKJr].type != 3)
							{
								if (positionDKJr == 1 || positionDKJr == 5 )
								{
									setGrilleJeu(3, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
									setGrilleJeu(2, positionDKJr, DKJR, pthread_self());
									afficherGrilleJeu();
									afficherDKJr(10, (positionDKJr * 2) + 7,7);
									etatDKJr = LIANE_BAS;

								}
								else if (positionDKJr == 7)
										{
											setGrilleJeu(3, positionDKJr);
											afficherGrilleJeu();
											effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
											setGrilleJeu(2, positionDKJr, DKJR, pthread_self());
											afficherGrilleJeu();
											afficherDKJr(10, (positionDKJr * 2) + 7,5);
											etatDKJr = DOUBLE_LIANE_BAS;
										}
										else if (positionDKJr == 2 || positionDKJr == 3 || positionDKJr == 4 || positionDKJr == 6)
										{
											setGrilleJeu(3, positionDKJr);
											afficherGrilleJeu();
											effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
											setGrilleJeu(2, positionDKJr, DKJR, pthread_self());
											afficherGrilleJeu();
											afficherDKJr(10, (positionDKJr * 2) + 7,8);
											pthread_mutex_unlock(&mutexGrilleJeu);
											nanosleep(&tempsjump, NULL);

											
											pthread_mutex_lock(&mutexGrilleJeu);
											if(grilleJeu[3][positionDKJr-1].type == CROCO)
											{
												pthread_mutex_lock(&mutexScore);
												score += 1;
												MAJScore = true;
												pthread_mutex_unlock(&mutexScore);
												pthread_cond_signal(&condScore);

											}
											setGrilleJeu(2, positionDKJr);
											afficherGrilleJeu();
											effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

											if(grilleJeu[3][positionDKJr].type != 2)
											{
												setGrilleJeu(3, positionDKJr, DKJR, pthread_self());
												afficherGrilleJeu();
												afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
											}
											else
											{
												pthread_mutex_unlock(&mutexGrilleJeu);
												pthread_mutex_unlock(&mutexEvenement);

												pthread_kill((grilleJeu[3][positionDKJr].tid),SIGUSR2);
												printf("poss dkjr : %d tid : %u\n",positionDKJr,grilleJeu[2][positionDKJr].tid);
												pthread_exit(0);
											}
											
										}

							}
							else
							{
								setGrilleJeu(3, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
								pthread_mutex_unlock(&mutexGrilleJeu);
								pthread_mutex_unlock(&mutexEvenement);
								printf("poss dkjr : %d tid : %u\n",positionDKJr,grilleJeu[2][positionDKJr].tid);
								pthread_kill((grilleJeu[2][positionDKJr].tid),SIGUSR1);
								printf("poss dkjr : %d tid : %u\n",positionDKJr,grilleJeu[2][positionDKJr].tid);
								pthread_exit(0);
							}
							break;
					}
				case LIANE_BAS:
						if(evenement == SDLK_DOWN)
						{
							if(grilleJeu[3][positionDKJr].type != 2)
							{
									if(positionDKJr == 1)
									{
										setGrilleJeu(2, positionDKJr);
										afficherGrilleJeu();
										effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
										setGrilleJeu(3, positionDKJr, DKJR, pthread_self());
										afficherGrilleJeu();
										afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
										etatDKJr = LIBRE_BAS;

									}
									else if(positionDKJr == 5)
												{
													setGrilleJeu(2, positionDKJr);
													effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
													//positionDKJr++;
													setGrilleJeu(3, positionDKJr, DKJR, pthread_self());
													afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
													etatDKJr = LIBRE_BAS;

												}
							}
							else
							{
								setGrilleJeu(2, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
								pthread_mutex_unlock(&mutexGrilleJeu);
								pthread_mutex_unlock(&mutexEvenement);
								pthread_kill((grilleJeu[3][positionDKJr].tid),SIGUSR2);
			
								pthread_exit(0);
							}
						}
				break;
				case DOUBLE_LIANE_BAS:
					if(evenement == SDLK_UP)
							{
									
									setGrilleJeu(2, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

									if(grilleJeu[1][positionDKJr].type == CROCO)
									{
										kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
										pthread_mutex_unlock(&mutexEvenement);
										pthread_mutex_unlock(&mutexGrilleJeu);
										pthread_exit(0);
									}

									setGrilleJeu(1, positionDKJr, DKJR, pthread_self());
									afficherGrilleJeu();
									afficherDKJr(7, (positionDKJr * 2) + 7,6);
									etatDKJr = LIBRE_HAUT;

							}
						else 
				if(evenement == SDLK_DOWN)
						{
								if(grilleJeu[3][positionDKJr].type != 2)
								{
									setGrilleJeu(2, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
									setGrilleJeu(3, positionDKJr, DKJR, pthread_self());
									afficherGrilleJeu();
									afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
									etatDKJr = LIBRE_BAS;		
								}
								else
								{
									setGrilleJeu(2, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_kill((grilleJeu[3][positionDKJr].tid),SIGUSR2);
				
									pthread_exit(0);
								}
						}
				//...
				break;
				case LIBRE_HAUT:
				switch (evenement)
					{
							case SDLK_LEFT:

							if (positionDKJr == 3)
							{
								setGrilleJeu(1, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(7, (positionDKJr * 2) + 7,2 ,2);
								setGrilleJeu(0, positionDKJr, DKJR, pthread_self());
								setGrilleJeu(0, positionDKJr);
								positionDKJr--;
								afficherGrilleJeu();
								setGrilleJeu(0, positionDKJr);
								afficherGrilleJeu();
								afficherDKJr(5, (positionDKJr * 2) + 7,9);
								
								if(grilleJeu[0][1].type == 4)
								{
									nanosleep(&tempsanim, NULL);
									effacerCarres(5, (positionDKJr * 2)+1 + 7, 4,4);


									setGrilleJeu(0, positionDKJr, DKJR , pthread_self());
									afficherGrilleJeu();
									afficherDKJr(4, ((positionDKJr+3) * 2) + 7,10);

									
									setGrilleJeu(0, positionDKJr);
									afficherGrilleJeu();
									nanosleep(&tempsanim, NULL);
									effacerCarres(2, (positionDKJr * 2) + 7, 4,4);

									pthread_mutex_lock(&mutexScore);
									score += 10;
									MAJScore = true;
									pthread_mutex_unlock(&mutexScore);
									pthread_cond_signal(&condScore);


									pthread_mutex_lock(&mutexDK);
									MAJDK = true;
									pthread_mutex_unlock(&mutexDK);
									pthread_cond_signal(&condDK);
									
									afficherDKJr(6, ((positionDKJr+3) * 2) + 7,11);
									nanosleep(&tempsanim, NULL);
									effacerCarres(6, ((positionDKJr * 2)-1) + 7,4,4);
									if(grilleJeu[3][1].tid !=0)
										pthread_kill(grilleJeu[3][1].tid,SIGUSR2);
									if(grilleJeu[3][2].tid !=0)
										pthread_kill(grilleJeu[3][2].tid,SIGUSR2);
									if(grilleJeu[3][3].tid !=0)
										pthread_kill(grilleJeu[3][3].tid,SIGUSR2);
									if(grilleJeu[2][0].tid !=0)
										pthread_kill(grilleJeu[2][0].tid,SIGUSR1);
									if(grilleJeu[2][1].tid !=0)
										pthread_kill(grilleJeu[2][1].tid,SIGUSR1);
									if(grilleJeu[2][2].tid !=0)
										pthread_kill(grilleJeu[2][2].tid,SIGUSR1);

									positionDKJr = 1;
									afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
									setGrilleJeu(3, 1, DKJR , pthread_self());
									etatDKJr = LIBRE_BAS;
									afficherGrilleJeu();
								}
								else		
								{
									nanosleep(&tempsanim, NULL);

									effacerCarres(5, (positionDKJr * 2)+1 + 7, 4,4);

									setGrilleJeu(0, positionDKJr, DKJR , pthread_self());
									afficherGrilleJeu();
									afficherDKJr(6, (positionDKJr * 2) + 7,12);
									nanosleep(&tempsanim, NULL);
									effacerCarres(6, (positionDKJr * 2) + 7, 4,4);
									afficherDKJr(11, (positionDKJr * 2) + 7,13);
									setGrilleJeu(0, positionDKJr);
									afficherGrilleJeu();
									nanosleep(&tempsanim, NULL);
									positionDKJr = 0;
									effacerCarres(11, (positionDKJr * 2) + 7, 2,2);

									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_exit(0);
									/*positionDKJr = 0;
									afficherDKJr(11, (positionDKJr * 2) + 7,13);
									etatDKJr = LIBRE_BAS;*/
								}
							}

							if (positionDKJr > 3)
							{
								setGrilleJeu(1, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr--;
								if(grilleJeu[1][positionDKJr].type == CROCO)
								{
									kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_exit(0);
								}
								setGrilleJeu(1, positionDKJr, DKJR , pthread_self());
								afficherGrilleJeu();
								afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							}
							break;

							case SDLK_RIGHT:
							if (positionDKJr < 7)
							{
								setGrilleJeu(1, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr++;
								if(grilleJeu[1][positionDKJr].type == CROCO)
								{
									kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_exit(0);
								}
								setGrilleJeu(1, positionDKJr, DKJR, pthread_self());
								afficherGrilleJeu();
								afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							}
							
							break;
							case SDLK_UP:
							if (positionDKJr == 3 || positionDKJr == 4 )
							{
								setGrilleJeu(1, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(0, positionDKJr, DKJR , pthread_self());
								afficherGrilleJeu();
								afficherDKJr(6, (positionDKJr * 2) + 7,8);

								pthread_mutex_unlock(&mutexGrilleJeu);
								nanosleep(&tempsjump, NULL);
								pthread_mutex_lock(&mutexGrilleJeu);
								if(grilleJeu[1][positionDKJr+1].type == CROCO)
											{
												pthread_mutex_lock(&mutexScore);
												score += 1;
												MAJScore = true;
												pthread_mutex_unlock(&mutexScore);
												pthread_cond_signal(&condScore);

											}
								setGrilleJeu(0, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

								if(grilleJeu[1][positionDKJr].type != 2)
								{
									
									setGrilleJeu(1, positionDKJr, DKJR, pthread_self());
									afficherGrilleJeu();
									afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
									etatDKJr = LIBRE_HAUT;	
								}
								else
								{
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_kill((grilleJeu[1][positionDKJr].tid),SIGUSR2);
				
									pthread_exit(0);
								}


							}
							if (positionDKJr == 6 )
							{
								setGrilleJeu(1, positionDKJr);
								afficherGrilleJeu();
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(0, positionDKJr, DKJR, pthread_self());
								afficherGrilleJeu();
								afficherDKJr(6, (positionDKJr * 2) + 7,7);
								etatDKJr = LIANE_HAUT;


							}
							
								break;
							case SDLK_DOWN:

								if(positionDKJr == 7)
								{

										setGrilleJeu(1, positionDKJr);
										afficherGrilleJeu();
										effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

										afficherDKJr(10, (positionDKJr * 2) + 7,5);

										setGrilleJeu(2, positionDKJr, DKJR, pthread_self());
										afficherGrilleJeu();

										etatDKJr = DOUBLE_LIANE_BAS;
								}
								break;
					}
				
				
				//...
				break;
				case LIANE_HAUT:
				if(evenement == SDLK_DOWN)
							{

								if(grilleJeu[1][positionDKJr].type != 2)
								{
									setGrilleJeu(0, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
									setGrilleJeu(1, positionDKJr, DKJR, pthread_self());
									afficherGrilleJeu();
									afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
									etatDKJr = LIBRE_HAUT;	
								}
								else
								{
									setGrilleJeu(0, positionDKJr);
									afficherGrilleJeu();
									effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_kill((grilleJeu[1][positionDKJr].tid),SIGUSR2);
				
									pthread_exit(0);
								}


							}
				break;
		}

		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}




void* FctThreadDK(void* p)
{
	struct timespec temps;

	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;
	int i = 4;
	pthread_cond_init(&condDK, NULL);


 while(1)
 {
 		if(i==4)
 		{
 			afficherCage(1);
	 		afficherCage(2);
	 		afficherCage(3);
	 		afficherCage(4);
 		}
 		if(i==3)
 		{

	 		afficherCage(2);
	 		afficherCage(3);
	 		afficherCage(4);

 		}
 		if(i==2)
 		{
	 		afficherCage(3);
	 		afficherCage(4);
 		}
 		if(i==1)
 		{
	 		afficherCage(4);
 		}


 		
 		printf("test : %d\n",i);
 		pthread_mutex_lock(&mutexDK);
		while (MAJDK == false)
		pthread_cond_wait(&condDK, &mutexDK);
		pthread_mutex_unlock(&mutexDK);
		effacerCarres(2,7,4,4);
		
		i--;
		MAJDK = false;
		if(i==0)
		{
			afficherRireDK();
			nanosleep(&temps,NULL);
			effacerCarres(2,7,4,4);
			i=4;
			pthread_mutex_lock(&mutexScore);
			score += 10;
			MAJScore = true;
			pthread_mutex_unlock(&mutexScore);
			pthread_cond_signal(&condScore);
		}

	}
	
	pthread_exit(0);
}	


void* FctThreadScore(void* p)
{


	 while(1)
	 {

	 		pthread_mutex_lock(&mutexScore);
	 		pthread_once(&controleur,afficherS);
	 		afficherScore(score);
			while (MAJScore == false)
			pthread_cond_wait(&condScore, &mutexScore);
			if(score % 300 == 0)
			{
				//pthread_mutex_lock(&mutexdelaiEnnemis);
				delaiEnnemis = 3000;

				effacerCarres(7,26,1,3);
				if(VIE_DKJR>0)
				{
					VIE_DKJR--;
	
					afficherEchec(VIE_DKJR);
				}
				
					
				
				pthread_mutex_unlock(&mutexdelaiEnnemis);
			}
			MAJScore = false;
	 		afficherScore(score);
			pthread_mutex_unlock(&mutexScore);
		}
		pthread_exit(0);
}


void* FctThreadEnnemis(void * p)
{
	char choix;
	int rett;
	sigset_t mask;
	struct timespec tempsalarm;

	sigfillset(&mask);
	sigdelset(&mask, SIGALRM);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	



	alarm(15);

	while(1)
	{
		pthread_mutex_lock(&mutexdelaiEnnemis);
		tempsalarm.tv_sec = delaiEnnemis/1000;
		tempsalarm.tv_nsec = delaiEnnemis%1000 * 1000000;
		pthread_mutex_unlock(&mutexdelaiEnnemis);

		printf("delai : %d \n",delaiEnnemis);

		srand(time(NULL));
	  choix = rand() % 2;

	  if(choix == 0)
	  {
	  	rett = pthread_create(&threadCorbeau, NULL,(void*(*)(void*)) FctThreadCorbeau,(void *) NULL);
	  }
	  else
	  {
	  	rett = pthread_create(&threadCroco, NULL,(void*(*)(void*)) FctThreadCroco,(void *) NULL);
  	}

  	nanosleep(&tempsalarm, NULL);
  }

  pthread_exit(0);

}

void* FctThreadCorbeau(void * p)
{

	sigset_t mask;

	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, NULL);


	struct timespec temps;

	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;
	int * positionHorizontale = (int*)malloc(sizeof(int));
	int houb = 0;

	*positionHorizontale = 0;
	while(*positionHorizontale < 8)
	{
		pthread_mutex_lock(&mutexGrilleJeu);

		if (houb == 0)
		{
			afficherCorbeau(*positionHorizontale*2+8,2);
			houb++;
		}
		else if (houb == 1)
		{
			afficherCorbeau(*positionHorizontale*2+8,1);
			houb--;
		}
		if(pthread_setspecific(keySpec, positionHorizontale))
		{
			puts("!!!! Erreur de setspecific");
		}


		setGrilleJeu(2,*positionHorizontale,CORBEAU,pthread_self());
		printf("tid du CROCO : %u\n",pthread_self());
		afficherGrilleJeu();
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps ,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);


		setGrilleJeu(2,*positionHorizontale);
		afficherGrilleJeu();
		
		effacerCarres(9, (*positionHorizontale * 2) + 8, 2, 2);
		if((grilleJeu[2][(* positionHorizontale+1)].type)==1)
		{
			pthread_kill(grilleJeu[2][(* positionHorizontale+1)].tid,SIGINT);
			pthread_mutex_unlock(&mutexGrilleJeu);
			pthread_exit(0);
		}
		else
		* positionHorizontale += 1;
		pthread_mutex_unlock(&mutexGrilleJeu);
	}

	pthread_exit(0);
}

void* FctThreadCroco(void * p)
{

	sigset_t mask;

	sigfillset(&mask);
	sigdelset(&mask, SIGUSR2);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	

	struct timespec temps;

	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;
	S_CROCO * SCP = (S_CROCO*)malloc(sizeof(S_CROCO));
	int houb = 0;

	SCP->position = 2;
	SCP->haut = true;
	while(SCP->position < 8)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		if (houb == 0)
		{
			afficherCroco(SCP->position*2+7,2);
			houb++;
		}
		else if (houb == 1)
		{
			afficherCroco(SCP->position*2+7,1);
			houb--;
		}

		setGrilleJeu(1,SCP->position,CROCO,pthread_self());
		printf("tid du CROCO : %u\n",pthread_self());
		afficherGrilleJeu();

		if(pthread_setspecific(keySpec, SCP))
		{
			puts("!!!! Erreur de setspecific");
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps ,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);


		setGrilleJeu(1,SCP->position);
		afficherGrilleJeu();
		
		effacerCarres(8, (SCP->position * 2) + 7, 1, 1);
		if(((grilleJeu[1][(SCP->position+1)].type)==1) )
		{

			pthread_kill(grilleJeu[1][(SCP->position+1)].tid,SIGHUP);

			pthread_mutex_unlock(&mutexGrilleJeu);
			pthread_exit(0);
		}
		else
		SCP->position += 1;
		pthread_mutex_unlock(&mutexGrilleJeu);
	}

	SCP->haut = false;
	afficherCroco(SCP->position*2+8,3);
	pthread_mutex_unlock(&mutexGrilleJeu);
	nanosleep(&temps ,NULL);
	pthread_mutex_lock(&mutexGrilleJeu);
	effacerCarres(9, (SCP->position* 2) + 7, 2, 2);
	if((grilleJeu[3][(SCP->position-1)].type)==1 || (grilleJeu[3][(SCP->position)].type)==1)
		{
			
			pthread_kill(grilleJeu[3][(SCP->position-1)].tid,SIGCHLD);
			pthread_mutex_unlock(&mutexGrilleJeu);
			pthread_exit(0);
		}
	SCP->position -= 1;

	houb = 0;
	pthread_mutex_unlock(&mutexGrilleJeu);
	while(SCP->position >0)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		if (houb == 0)
		{
			afficherCroco(SCP->position*2+8,5);
			houb++;
		}
		else if (houb == 1)
		{
			afficherCroco(SCP->position*2+8,4);
			houb--;
		}

		setGrilleJeu(3,SCP->position,CROCO,pthread_self());
		printf("tid du CROCO : %u\n",pthread_self());
		afficherGrilleJeu();

		if(pthread_setspecific(keySpec, SCP))
		{
			puts("!!!! Erreur de setspecific");
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps ,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(3,SCP->position);
		afficherGrilleJeu();
		effacerCarres(12, (SCP->position * 2) + 8, 2, 2);
		if((grilleJeu[3][(SCP->position-1)].type)==1)
		{

			pthread_kill(grilleJeu[3][(SCP->position-1)].tid,SIGCHLD);
			pthread_mutex_unlock(&mutexGrilleJeu);

			pthread_exit(0);
		}
		else
		SCP->position -= 1;
		pthread_mutex_unlock(&mutexGrilleJeu);
	}
	printf("CROCO CREE\n");	
	pthread_exit(0);
}

void HandlerSIGQUIT(int)
{
	printf("SIGQUIT RECU\n");	
}

void HandlerSIGALRM(int)
{
	printf("SIGALARM RECU\n");	
	pthread_mutex_lock(&mutexdelaiEnnemis);
	if((delaiEnnemis - 250) >= 2500)
	{
		delaiEnnemis =  delaiEnnemis-250;
		alarm(15);
	}
	pthread_mutex_unlock(&mutexdelaiEnnemis);
}

void HandlerSIGUSR1(int)
{

	int * positionHorizontale ;

	positionHorizontale = (int*)pthread_getspecific(keySpec);

	setGrilleJeu(2,*positionHorizontale);
	afficherGrilleJeu();
	effacerCarres(9, (*positionHorizontale * 2) + 8, 2, 2);
	
	printf("(Reception d'un signal HandlerSIGUSR1)   \n");
	pthread_exit(0);

	

}

void HandlerSIGUSR2(int)
{

	S_CROCO * SCP ;

	SCP = (S_CROCO*)pthread_getspecific(keySpec);


	if(SCP->haut == true)
	{
		setGrilleJeu(1,SCP->position);
		afficherGrilleJeu();
		effacerCarres(8, (SCP->position * 2) + 7, 2, 2);
	}
	else
	{
		setGrilleJeu(3,SCP->position);
		afficherGrilleJeu();
		effacerCarres(12, (SCP->position * 2) + 8, 2, 2);
	}

	
	printf("(Reception d'un signal HandlerSIGUSR2)   \n");
	pthread_exit(0);

	

}

void HandlerSIGCHLD(int)
{


	setGrilleJeu(3,positionDKJr);
	afficherGrilleJeu();
	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

	
	printf("(Reception d'un signal HandlerSIGUSR2)   \n");
	pthread_exit(0);

	

}

void HandlerSIGHUP(int)
{
	setGrilleJeu(1,positionDKJr);
	afficherGrilleJeu();
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

	
	printf("(Reception d'un signal HandlerSIGUSR2)   \n");
	pthread_exit(0);
	
}


void HandlerSIGINT(int)
{


	setGrilleJeu(2,positionDKJr);
	afficherGrilleJeu();
	effacerCarres(9, (positionDKJr * 2) + 7, 4, 2);
	
	printf("(Reception d'un signal HandlerSIGUSR1)   \n");


	if(etatDKJr == LIBRE_BAS)
		pthread_mutex_unlock(&mutexEvenement);
	pthread_exit(0);

	

}


void DestructeurVS(void *p)
{
	puts("=== liberation d'une zone specifique ===");
	free(p);
}


void afficherS()
{
	afficherScore(score);
}