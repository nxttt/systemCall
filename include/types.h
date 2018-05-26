#ifndef TYPES_H
#define TYPES_H

/// @file

/// @{

/**
 *	@brief	Struttura che salva chi sta esaminando quale stringa
 */
struct Status{
	int grandson;	///< Il nipote che sta esaminando la stringa
	int id_string;	///< Il numero della stringa che sta venendo esaminata
};

/**
 *	@brief	Struttura utilizzata per scrivere/leggere dalla coda di messaggi
 */
struct Message{
	long mtype;	///< Quit signal : 1, ogni altro valore intero >1 scrive il messaggio
	char text[128];	///< Buffer per la stringa da scrivere
};

/**
 *	@brief	Struttura per passare id e l' indirizzo della memoria condivisa
 */
struct Shmem{
	int shmid;	///< Id memoria condivisa
	void* addr; 	///< Spazio indirizzamento
};

/**
 *	@brief	Struttura per leggere dal file di input il plain_text e encoded_text in load_string
 */
struct Spek{
	char * plain_text[256];
	char * encoded_text[256];
};

/// @}

#endif
