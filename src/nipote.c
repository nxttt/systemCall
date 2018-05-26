#include "../include/nipote.h"
#include "../include/types.h"
#include "../include/utili.h"

#define SEMKEY 14
#define MSGKEY 90
#define SHMKEYS1 7777
#define SHMKEYS2 8888


// Restituisce "chiave trovata in n"
char * formattaBene(double n);





// VARIABILI GLOBALI





// id shared memory s1 - id shared memory s2 - id del nipote (1 o 2) - id della coda di messaggi
void nipote(int shmid1, int shmid2, int nstrings, int x, int msgid){
	int my_string;
	int sem, shm;
	struct Status *p;
	struct Spek *stringa;
	void * addr;
	//char * plain_text[256];
	//char * encoded_text[256]; 
	char *plain_text = (char *)malloc(sizeof(char)*256);
	char *encoded_text = (char *)malloc(sizeof(char)*256);
	
	//dovrebbe essere in un while(?)
	
	sem = lock();

	//ora sono nella zona in cui posso usare la memoria condivisa
	//la attacco al mio spazio di indirizzamento
	
	
	if(( addr = shmat(shmid1, 0, SHM_R) ) == (void*) -1){
		perror("shmat");
		//sblocco il semaforo
		sem = unlock();
		exit(-1);
	}
	//casting
	p = (struct Status *)addr;
	my_string = p->id_string;
	
	// Se ci sono ancora stringhe da esaminare
	if(my_string != nstrings){
		//memorizza nel campo Grandson l'id del nipote
		p->grandson = x; //o getpid()?
		p->id_string += 1;
		stringa = load_string(my_string, addr); //carico la stringa da addr
		kill(getppid(), SIGUSR1);
		//unlock()
		sem = unlock();
	}
	else{
		//esco dall'accesso esclusivo 
		//unlock()
		sem = unlock();
		
		//termina
		if(( shm = shmdt(p) ) == -1){
			perror("shmdet");
			exit(-1);
		}
		exit(0);		
	}
	
	// Lo faccio metre ho l'accesso esclusivo
	// struct spek *stringa = load_string(mystring, addr); //carico la stringa da addr

	copiaStrArr2(plain_text, stringa->plain_text);
	copiaStrArr2(encoded_text, stringa->encoded_text);

	//trova la chiave
	//e conta i secondi
	
	// Da rimpiazzare con systemCalls (?)
	clock_t start = clock();
	int seconds = 0;
	unsigned key = find_key(plain_text, encoded_text);
	clock_t end = clock();
	double timeelapsed = (double) (start - end) / CLOCKS_PER_SEC;

	//salvo key in S2
	
	// Serve un altro semaforo?
	
	void *shm2;
	if(( shm2 = shmat(shmid2, 0, SHM_W) ) == (void*) -1){
		perror("shmat");
		exit(-1);
	}
	
	save_key(my_string, shm2, key);
	
	int coda = msgget(MSGKEY, 0666);
	
	/*
		deposita il messaggio “chiave
		trovata/secondi” nella coda di messaggi
		del processo logger
	*/
	send_timeelapsed(formattaBene(timeelapsed), coda); 

	
}

struct Spek* load_string(int mystring, void * addr){
	
	// Struttura in cui salvero'
	struct Spek *stringa = (struct Spek*) malloc(sizeof(struct Spek));

	int j;
	if(mystring != 0){ // vado a prendere la stringa giusta
		j = mystring;
		// casting
		char *p = (char *)(addr+sizeof(struct Status));
		while(j != 0){
			// char *p = addr+sizeof(Status);
			while(*p != '\n') p++; //mi sposto fino alla fine della j-esima stringa
			j = j-1; //se ci sono altre stringhe ripeto la procedura
		}
		addr = p;
	}
	else //se è la prima (numero 0) mi sposto solo di struct status
		addr += sizeof(struct Status);

	char *plain_text = (char *)malloc(sizeof(char)*256);
	char *encoded_text = (char *)malloc(sizeof(char)*256);
	char * c;
	// j = 1 perche' il primo carattere e' <
	j = 1;
	//casting
	c = (char *)addr;
	
	// Finche non trovo '>' (dovrebbe essere ';' )
	while(*(c+j) != '>'){
		// j-1 per l'offset
		*(stringa->plain_text[j-1]) = *(c+j);
		++j;
	}	
	// + 2, perche' c ha j-1 plain, + ';<'
	c = c+j+2;
	// o facciamo aggiungiamo 3 sopra e j=0, oppure facciamo partire j da 1 (con j=0 c punterebbe a '<'  
	j = 1;
	
	// Finche' non trovo '>'
	// Controllare anche che non sia la fine del file? no
	while(*(c+j) != '>'){
		// j-1 per offset
		*(stringa->encoded_text[j-1]) = *(c+j);
		++j;
	}
	
	
	//stringa->plain_text = plain_text;
	//stringa->encoded_text = encoded_text;
	return stringa;
}

int lock(){
	struct sembuf* s = (struct sembuf *) malloc(sizeof(struct sembuf));
	s->sem_num = 0; //un solo semaforo
	s->sem_flg = 0; //nessun flag (così se arriva a 0 si blocca)
	s->sem_op = -1; //decremento il semaforo per utilizzarlo
	int sem;
	if((sem = semop(SEMKEY, s, 1)) == -1){
		perror("Semop");
		exit(-1);
	}
	return sem;
}

int unlock(){
	struct sembuf* s = (struct sembuf *) malloc(sizeof(struct sembuf));
	s->sem_num = 0; 
	s->sem_flg = 0;
	s->sem_op = +1;
	int sem;
	if((sem = semop(SEMKEY, s, 1)) == -1){
		perror("semop");
		exit(-1);		
	}
	return sem;
}

unsigned find_key(char * plain_text, char * encoded_text){
	unsigned* plain_unsigned = (unsigned*) plain_text;
	unsigned* encoded_unsigned = (unsigned*) encoded_text;
	unsigned key;
	
	
	while(*plain_unsigned ^ key != *encoded_unsigned) key++;
	return key;
}

void send_timeelapsed(char * string, int coda){
	int messaggio; 
	struct Message *m;

	/* allocazione memoria per il messaggio */
	m = (struct Message *) malloc(sizeof(struct Message));
	
	const unsigned msg_size = sizeof(struct Message) - sizeof(long);
	/* creazione del messaggio da inviare */
	m->mtype = 2;
	copiaStr(m->text, string);
	
	// Era IPC_NOWAIT
	if((messaggio = msgsnd(coda, m, msg_size, 0)) == -1){
		perror("msgsnd");
		free(m);	// free della malloc
		exit(-1);
	}
	free(m);	// free della malloc
}

// Cambiarla se funziona il passaggio di s1 ed s2
// void save_key(int mystring, void shmid2, unsigned key)
void save_key(int mystring, void *shm2, unsigned key){
	
	// Non va bene, dato che il nipote eseguira' piu' di una volta questa funzione, attaccheremmo piu' volte la stessa memoria
	// Inoltre se il passaggio di s2 funziona non e' necessario
	/*
	void *shm2;
	if(( shm2 = shmat(shmid2, 0, SHM_W) ) == (void*) -1){
		perror("shmat");
		exit(-1);
	}
	*/
	
	// Casting
	unsigned *in = (unsigned *)shm2;
	if(mystring != 0){ // vado a prendere la posizione giusta scorrendo con 'in'
		int k = mystring;	
		while(k != 0){
			//	Dobbiamo trasformare la chiave in esadecimale e poi in stringa
			in += sizeof(unsigned);
			k = k-1; //se ci sono altre stringhe ripeto la procedura
		}
	}
	*in = key;
}



// Cambiato per tenere conto del fatto che n potrebbe avere piu' di una cifra
// Non stiamo contando i decimi di secondo...
char * formattaBene(double n){
	char * string = "chiave trovata in ";
	int i = (int) n;
	int cif = countCifre(i);
	char c[cif];
	intToChar(i, cif, c);
	// controllare l' interazione con i vari '\0'
	char * k = (char *)malloc(sizeof(string)+sizeof(c));
	copiaStr(k, string);
	copiaStr(&k[18], c);
	
	//while(*(k++) != '\0');
	//*(k-1) = n + '0';
	//return string;
	return k;
}
