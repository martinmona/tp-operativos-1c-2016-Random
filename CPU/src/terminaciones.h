#ifndef TERMINACIONES_H_
#define TERMINACIONES_H_


#include <unistd.h>
#include <commons/config.h>
#include "protocolo.h"
#include "common_sockets.h"
#include "estructuras.h"
#include "common_sockets.h"
#include "CPU.h"
#include "funcionesCPU.h"
#include "paquete.h"
#include "primitivas.h"
#include "stack.h"


void terminaFinQuantum(void* dataExtra, t_size extraSize);
void terminaExcepcion(void* dataExtra, t_size extraSize);
void terminaBloqueoIO(void* dataExtra, t_size extraSize);
void terminaBloqueoDeSemaforos(void* dataExtra, t_size extraSize);
void terminaProcesoFinalizado(void* dataExtra, t_size extraSize);
void terminarCPUSinPCB();
void terminarCPU(void* dataExtra, t_size size);
t_envio* empaquetarPCB(t_PCB* pcbTmp, int codOp);

#endif /* TERMINACIONES_H_ */
