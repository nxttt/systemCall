#ifndef UTILI_H
#define UTILI_H



// Conta le cifre di un intero
int countCifre(int i);

//Copia stringa
void copiaStr(char *dest, const char *src);

//Copia stringa
void copiaStrArr2(char *dest, char **src);

//Copia stringa
void copiaStrArr1(char **dest, char *src);

//Trasforma un intero in una stringa
char * intToChar(int i, int cif, char *c);

int fileSize(char * file);

char * hex(int n);

#endif
