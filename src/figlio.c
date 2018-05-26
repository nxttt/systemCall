#include "../include/figlio.h"
#include "../include/types.h"
#include "../include/nipote.h"
#include "../include/utili.h"

#define MSGKEY   90
#define SHMKEYS1 7777
#define SHMKEYS2 8888

#define SEMKEY 14

//						CONSEGNA

//deposita il messaggio di terminazione nella coda di messaggi del processo logger
void send_terminate(int msgid);

//signal handler del segnale SIGUSR1
void status_updated(int currentSignal);




//						VARIABILI GLOBALI

int shmid1;					//id della shmem s1
int shmid2;
int msgid;
int size;						//size della struttura Status
char *shm;					//shmem address
struct Status *st; 	//puntatore allo status in memoria

pid_t pid1, pid2;		//pid per nipoti
int semid;					//id del semaforo


//wrapper del processo figlio, riceve in input la size del buffer s1

//CONTROLLARE COME LA FORK GESTISCE L'ATTACHED MEMORY DEL PADRE
void figlio(struct Shmem* s1, int s1Size, struct Shmem* s2, int s2Size, int nStrings){
	
	if((msgid = msgget(MSGKEY, 0666)) == -1) {
		perror("Coda di messaggi non esistente");
		exit(1);
	}
	
	int status1, status2;
	
	// Questa parte sotto non serve se la memoria attached e' condivisa tra padre e figlio
	
	/*
	//Setup zona condivisa
	size = sizeof(struct Status);
	//Ricerca zona condivisa
	if ((shmid1 = shmget(SHMKEYS1, s1Size, 0666)) < 0) {
        perror("shmget");
        exit(1);
  if ((shmid2 = shmget(SHMKEYS2, s2Size, 0666)) < 0) {
        perror("shmget");
        exit(1);
  }//Attach zona condivisa
	if ((shm = shmat(shmid1, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
  }
	*/
	
	
	st = (struct Status *) (s1->addr);
			
	//Creazione semaforo per figli
	int semid;
	
	struct sembuf* sops = (struct sembuf *) malloc(sizeof(struct sembuf));
	if ((semid = semget(SEMKEY, 2,IPC_CREAT|IPC_EXCL|0666)) == -1) {
		perror ("semget");
		free((struct sembuf*)sops);//libero malloc semaforo
		exit (1);
	}
	
	//setta il semaforo a 1 (mutex)
	sops->sem_num = 0;
	sops->sem_op = 1;
	sops->sem_flg = 0;
	
	semop(semid, sops, 1);
	
	//si registra per SIGUSR1
	signal(SIGUSR1, status_updated);
	
	/*
		Crea i vari nipoti
		Se La memoria condivisa e' condivisa tra padre e figli, basta passargli la struttura s1/s2 (?)
	*/
	pid1 = fork(); //Primo nipote
	if(pid1 == -1){
		perror("nipote1");
		free((struct sembuf*)sops);//libero malloc semaforo
		exit(1);
	}
	
	if(pid1 == 0){	//Se sono il nipote appena creato
		nipote(shmid1, shmid2, nStrings, 1, msgid);
		//nipote(shmid1, shmid2, nStrings, 1, semid, msgid);
		exit(0);
	}		
	//se sono il padre
	pid2 = fork(); //Secondo nipote
	if(pid2 == -1){
		perror("nipote2");
		free((struct sembuf*)sops);//libero la malloc
		exit(1);
	}
	
	if(pid2 == 0){	//Se sono il secondo nipote appena creato
		nipote(shmid1, shmid2, nStrings, 2, msgid);
		//nipote(shmid1, shmid2, nStrings, 2, semid, msgid);
		exit(0);
	}
	
	waitpid(pid1, &status1, 0);
	if(WEXITSTATUS(status1) == 1){
		write(1, "Nipote 1 crashato.\n", 19);
	}
	
	waitpid(pid2, &status2, 0);
	if(WEXITSTATUS(status2) == 1){
		write(1, "Nipote 2 crashato.\n", 19);
	}
	
	//Detach della zona di memoria condivisa
	if(shmdt(shm) == -1){
		perror("shmdt");
		free((struct sembuf*)sops);//libero la malloc prima di uscire
		exit(1);
	}						
	printf("Detached\n");		//DEBUG
	
	//Rimozione semaforo
	semctl(semid, 0, IPC_RMID);
	free((struct sembuf*)sops);
		
	send_terminate(msgid);
}







//signal handler
void status_updated(int currentSignal){
	write(1, "Il nipote ", 10);
	int cif = countCifre(st->grandson);
	char c[cif];
	write(1, intToChar(st->grandson, cif, c), cif);
	write(1, " sta analizzando la ", 20);
	cif = countCifre(st->id_string);
	char d[cif];
	write(1, intToChar(st->id_string, cif, d), cif);
	write(1, "-esima stringa.\n", 16);
}

void send_terminate(int msgid){

	//int msgid;
	
	/*
		puntatore alla struttura che rappresenta il messaggio.
		Il messaggio è definito nel file types.h
	*/
	struct Message *m;

	/* allocazione memoria per il messaggio */
	m = (struct Message *) malloc(sizeof(struct Message));
	
	const unsigned msg_size = sizeof(struct Message) - sizeof(long);
	/* creazione del messaggio da inviare */
	m->mtype = 1;
	copiaStr(m->text, "ricerca conclusa");
	//Mando il messaggio
	if(msgsnd(msgid, m, sizeof(struct Message) - sizeof(m->mtype), 0) == -1) {
			perror("msgsnd");
			free((struct Message *)m);//libero la malloc prima di uscire
			exit(1);
	}
	free((struct Message *)m);//libero la malloc prima di uscire
}
