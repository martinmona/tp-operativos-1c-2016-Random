/*
 * consola.h
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

//define
#define FALSE 0
#define TRUE 1
#define HANDSHAKE 1
#define ENVIAR_ANSISOP 2
#define IMPRIMIR 20
#define IMPRIMIRTEXTO 21
#define CERRARCONSOLA 3
#define ERROR 4
#define FALTA_LUGAR_SWAP 15
#define PROGRAMA_ALMACENADO 16

//variables
int socket_nucleo;

//estructuras
typedef
struct configuracion{
	char* ip;
	char* puerto;
}t_configuracion;

typedef struct{
        int protocolo;
        int tamArchivo;
       char* ansiSOP;
}t_archivo;

typedef struct{
       int tamanio;
       char* contenido;
}t_contenido;



//prototipos

void empaquetarYenviarArchivo(t_archivo *archivo);

int tamArchivo(char * direccionArchivo);

char * obtenerContenidoAnSISOP(char * direccionArchivo);
#endif /* CONSOLA_H_ */
