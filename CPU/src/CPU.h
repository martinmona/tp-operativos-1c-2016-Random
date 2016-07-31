#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "primitivas.h"
#include <parser/parser.h>
#include "estructuras.h"
#include "funcionesCPU.h"
#include "protocolo.h"
#include "common_sockets.h"
#include "paquete.h"
#include "stack.h"

//bool procesoActivo = false;
int quantum;
int QUANTUM_SLEEP;
int tamanoPagina;


#endif /* CPU_H_ */
