/*
 * conexiones.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "FuncionesUMC.h"
#include "common_sockets.h"

//MENSAJES CON SWAP--------------------------------
#define HANDSHAKE 							1
#define SWAP_OK 							2
#define NUEVO_PROGRAMA_SWAP					3
#define	NUEVO_PROGRAMA_SWAP_SIN_ESPACIO		4
#define NUEVO_PROGRAMA_SWAP_OK				5
#define LEER_PAGINA_SWAP					6
#define LEER_PAGINA_SWAP_OK					7
#define ESCRIBIR_PAGINA_SWAP				8
#define ESCRIBIR_PAGINA_SWAP_OK				9
#define FIN_PROGRAMA_SWAP					10
#define FIN_PROGRAMA_SWAP_OK				11


//MENSAJES CON NUCLEO-------------------------------
#define NUEVO_KERNEL						12
#define NUEVO_KERNEL_OK						13
#define NUEVO_PROGRAMA						14
#define NO_HAY_ESPACIO_SWAP					15
#define NUEVO_PROGRAMA_OK					16
#define FIN_PROGRAMA						17
#define FIN_PROGRAMA_OK						18


//MENSAJES CON CPU---------------------------------
#define NUEVA_CPU							20
#define NUEVA_CPU_OK						21
#define CAE_CPU								19
#define CAMBIO_PROCESO_ACTIVO_CPU			23
#define CAMBIO_PROCESO_ACTIVO_CPU_OK 		23
#define NUEVO_PROGRAMA_NO_HAY_MARCOS		25
#define LEER_PAGINA_CPU						26
#define LEER_PAGINA_CPU_OK					26
#define ESCRIBIR_PAGINA_CPU					28
#define ESCRIBIR_PAGINA_CPU_OK				28

#define ERROR_CODIGO_INVALIDO				30


#include "FuncionesUMC.h"
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


int sockets[50];

int socketEscucha, socketActivo, socketSWAP;
fd_set descriptoresLectura;

void escucharConexion();
void atenderConexion();




#endif /* CONEXIONES_H_ */
