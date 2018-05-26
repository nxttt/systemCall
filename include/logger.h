#ifndef LOGGER_H
#define LOGGER_H

#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


/// @file

/// @defgroup logger

/// @{

/**
 *	@brief Wrapper del processo logger
 */
void logger();

/**
 *	@brief	Stampa la coda di messaggi su stdout
 *	@return 1 se ha ricevuto un mtype=1 (quit signal), 0 altrimenti
 */
int polling_receive();
 
/// @} 

#endif
