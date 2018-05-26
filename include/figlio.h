#ifndef FIGLIO_H
#define FIGLIO_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>		//malloc, exit
#include <stdio.h>		//perror, printf, sprintf
#include <wait.h>			//SIGUSR1, signal
#include <unistd.h>		//fork, pause, write

#include "./types.h"
//#include <string.h>		//strcpy

/// @file

/// @defgroup figlio

/// @{

/**
 *	@brief	Wrapper del processo figlio
 *	@param s1Size Dimensione del buffer s1
 */
void figlio(struct Shmem* s1, int s1Size, struct Shmem* s2, int s2Size, int nStrings);

/**
 *	@brief	Signal handler, stampa quale stringa sta venendo esaminata da quale nipote accedendo alla struttura Status presente in memoria condivisa
 *	@param	currentSignal Il segnale da gestire
 */
void status_updated(int currentSignal);

/**
 *	@brief	Deposita il messaggio di terminazione (mtype=1) nella coda di messaggi del processo logger
 */
void send_terminate();

/// @}

#endif
