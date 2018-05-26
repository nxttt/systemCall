#include "../include/padre.h"
#include "../include/types.h"
#include "../include/nipote.h"
#include "../include/figlio.h"
#include "../include/logger.h"
#include "../include/utili.h"

#define SHMKEYS1 7777
#define SHMKEYS2 8888
#define BUFSIZE 1024

// IDEA: invece di usare padre per contare quante stringhe ci sono da decodificare, potremmo usare splice nei nipoti con \n come divisore? 
//	status partirebbe da 0, dopo il primo split aggiungiamo 1 ad id, il prossimo nipote in esecuzione dovra' semplicemente usare di nuovo lo split saltando il primo divisore (quindi saltando il divisore numero "id")


//*/*/*/*//*/**/*/*/*/*/*/*/mettere nelle funzioni utili*/*/*/*/*/*/*/*/*/*/*/*/



//*/*/*/*//*/**/*/*/*/*/*/*/mettere nelle funzioni utili*/*/*/*/*/*/*/*/*/*/*/*/

unsigned load_key(int i, void * addr);


int numString; // quante stringhe ci sono da decodificare




void padre(char * file, char * fileOutput){
	struct Shmem * s1;
	struct Shmem * s2; //strutture in types.h
	
	pid_t pid1;
	pid_t pid2;	
	int status1;
	int status2;
	
	int s1Size, s2Size; //Dimensione delle memorie condivise
	s1Size = sizeof(struct Status) + fileSize(file);
	s2Size = numString*sizeof(unsigned);
	
	// ci pensa attach_segments a fare la malloc
	// dobbiamo controllare che sia andato tutto a buon fine!
	
	s1 = attach_segments(SHMKEYS1, s1Size );

	load_file(file, s1->addr); //void ? 
	
	// quanta memoria allocare ad s2? numero di chiavi per sizeof(unsigned)
	//s2 = attach_segments(SHMKEYS2, 32*sizeof(byte)); // quanta memoria ?
	s2 = attach_segments(SHMKEYS2, s2Size); // quanta memoria ?

	if( ( pid1 = fork() ) == -1 ){ //logger
		perror("Errore nella fork");
		exit(-1);
	}
	else if (pid1 == 0)
		logger();
	
	if( ( pid2 = fork() ) == -1 ){ //figlio
		perror("Errore nella fork");
		exit(-1);
	}
	else if (pid2 == 0)
		figlio(s1, s1Size, s2, s2Size, numString);
	
	waitpid(pid1, &status1, 0);
	if(WEXITSTATUS(status1) == 1){
		write(1, "Logger crashato.\n", 19);
	}
	
	waitpid(pid2, &status2, 0);
	if(WEXITSTATUS(status2) == 1){
		write(1, "Figlio crashato.\n", 19);
	}

	//int mystring = 3; // andrebbe fatta globale
	if(check_keys(0, numString, (unsigned*)s2->addr))
		save_keys(fileOutput, s2->addr);
	
	detach_segments(s1->shmid);
	detach_segments(s2->shmid);
	exit(0);
}



struct Shmem* attach_segments(key_t key, int size){ 
	struct Shmem *s; //struttura risultato definita in types.h
	s = (struct Shmem*)malloc(sizeof(struct Shmem));
	int shmid;
	
	if((shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0660)) == -1){
		// Se fallisce a creare la zona di memoria restituisce una struttura con id -1 e addr 0
		s->shmid = shmid;
		s->addr = (void*) 0;
		return s;
	}else{
		void* a;		
		a = shmat(shmid, 0, SHM_W);
		if( a == (void*) -1){
		// Se fallisce ad attaccare la memoria restituisce s->id 0 e s->addr -1
			s->shmid = 0;
			s->addr = a;
			return s;
		}else{
		// Se va tutto bene restituisce l' id e l'indirizzo dentro ad s 
			s->shmid = shmid;
			s->addr = a;
			return s;
		}
	}
}

// Perche' un while?
void detach_segments(int descrittore){
	while(!shmctl(descrittore, IPC_RMID, NULL));
}

void load_file(char * file, void* addr){ //ci sono da gestire gli errori
	int fd = open(file, O_RDWR, 0666);
	int size;

	//char * buf = (char*)malloc(BUFSIZE); // stringa con 1024 byte?
	//Inutile
	//struct Status *s;
	// s non e' stata ancora allocata
	//*addr = *s;
	// forse volevi:
	// Vogliamo mettere Status all'inizio della memoria, il che vuol dire che il contenuto del file di input deve essere scritto dopo status
	
	// dato che dobbiamo contare quante stringhe dobbiamo decodificare, leggiamo il file char per char, e se troviamo un \n allora aggiungiamo 1 a numString
	char buf; //buffer di un char
	//casting
	char *caddr = (char *) addr;
	
	caddr = caddr + sizeof(struct Status);
	do{
		//size = read(fd, buf, BUFSIZE); //leggo dal file
		size = read(fd, &buf, 1); //leggo dal file
		// read restituisce 0 se e' arrivata alla fine del file mentre leggeva? 
		// No, problem
		if(buf == '\n'){
			numString++;
		}
		if(size != 0){
				//strcpy(addr, buf); //copio quello che ho letto nella shmid
				//devo spostare addr nel caso ci sia ancora qualcosa da leggere
				//addr = addr + BUFSIZE;
				
				*caddr = buf;
				caddr++;				
		}		
	}while (size > 0); //finchè c'è da leggere, read restituisce i byte letti
	//free(buf);
	close(fd);
}


int check_keys(int i, const int max, void * addr){
	if( i < max ){
		// Cos'e' questa load_string()?
		struct Spek *string = load_string(i, addr);
		
		// Carica la chiave con id i
		unsigned key;
		key = load_key(i, addr);
		// Prima di questo string->plain_text deve essere trasformato in un unsigned, cosi' come encoded_text
		
		unsigned* plain_unsigned = (unsigned*) string->plain_text;
		unsigned* encoded_unsigned = (unsigned*) string->encoded_text;
		
		if(*(plain_unsigned) ^ key == *(encoded_unsigned)){
			return check_keys(i+1, max, addr);
		}else{
			return 0;
		}
	}	
	return 1;
}


void save_keys(char * fileOutput, void * addr){
	// Deve essere un file non esistente, dobbiamo crearlo noi
	int fd = open(fileOutput, O_RDWR, 0666);
	int j = 0;
	unsigned key;
	//char * keyHexed[32];
	char * keyHexed=(char *)malloc(32*sizeof(char));
	int wr;
	while(j <= numString){
		key = load_key(j, addr);
		copiaStr(keyHexed, hex(key));
		wr = write(fd, keyHexed, 32);
		j++;
	}
	close(fd);
}

//	EXTRA


// Dato l'indirizzo s2 trova la chiave con id i e la carica in key
unsigned load_key(int i, void * addr){
	unsigned key;
	unsigned * in;
	in = addr;
	
	while(i != 0){
		in += sizeof(unsigned);
		i = i-1;
	}
	
	// in deve puntare ad un unsigned
	key = *in;
	return key;	
}

/* JUST..NO
	Non e' con system calls, non gli passi il file descriptor e usi bufferizzazione
	
*/

