/*
 * conexiones.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "Nucleo.h"
#include "common_sockets.h"

//MENSAJES CON UMC--------------------------------
#define HANDSHAKE 							12
#define UMC_OK	 							13



//MENSAJES CON CONSOLA-----------------------------
#define NUEVA_CONSOLA						12
#define NUEVA_CONSOLA_OK					13



//MENSAJES CON CPU---------------------------------
#define NUEVA_CPU							20
#define NUEVA_CPU_OK						21


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int sockets[100];

int socketEscuchaCPU, socketEscuchaConsola, socketActivoCPU, socketActivoConsola, socketUMC;
fd_set descriptoresLecturaCPU;
fd_set descriptoresLecturaConsola;

void escucharConexionCPU();
void escucharConexionConsola();
void atenderConexionCPU();
void atenderConexionConsola();




#endif /* CONEXIONES_H_ */
