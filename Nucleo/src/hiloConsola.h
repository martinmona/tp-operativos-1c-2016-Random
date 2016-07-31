#ifndef HILOCONSOLA_H_
#define HILOCONSOLA_H_

#include "Nucleo.h"
#include "estructuras.h"
#include "Nucleo.h"




t_PCB* crearPCB(char* codigo, int tamanioArchivo, int sConsola);

void hiloConsola(void* socket);

#endif /* HILOCONSOLA_H_ */
