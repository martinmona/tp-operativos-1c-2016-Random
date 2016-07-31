/*
 * Swap.h
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_


#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <wait.h>
#include <errno.h>          /* errno, ECHILD            */
#include "common_sockets.h"

//define
#define FAIL -1
#define RECIBIRTAMANIO 1
#define FRAGMENTACION_EXTERNA -1
#define TRUE 1
#define FALSE 0
#define HANDSHAKE 2
#define SOLICITUD_DE_INCIO 3
#define NOHAYLUGAR 4
#define HAYLUGAR 5
#define SOLICITUD_LEER 6
#define DATOLEIDO 7
#define SOLICITUD_ESCRITURA 8
#define ESCRIBIOPAGINA 9
#define FIN_PROCESO 10
#define ELIMINOPROCESO 11

//variables
char* ipUMC;
char* umcPort;
char* puerto_Escucha;
char* nombreSwap;
int paginas;
int tamPagina;
int retCompactacion;
int tamArchivo;
char* archivoMappeado;
int retAcceso;

//estructuras
 typedef struct{
  	int inicio;
  	int tamanio;
  	bool bitMap;
  	int numDePag;
  	int pid;
  }espacio;


t_list * listaEspacios;
t_log* logger;
//prototipos

void cargarEspaciosLibres();
bool escribirPagina(int pid, int numeroDePagina, char*pagina);
char* leerUnaPagina(int pid, int numPagina);
void eliminarProceso(int pid);
void reservarPaginas(int paginaDeComienzo, int pid, int cantidadDePaginas);
void compactarEspacioLibre();
int paginasContiguas(int cantPaginas);
bool hayEspacio(espacio* esp);
bool recibirNuevoPrograma(int pid, int cantidadPaginas);
void crearListas();
espacio * crearEspacioLibre (int inicio);
void agregarEspacioLibre(int inicio);
espacio* crearEspacioAsignado (bool bitMap,int numDePag,int posicionDePag, int pid);
void agregarEspacioAsigando(bool bitMap,int numDePag,int posicionDePag, int pid);
void* mappearArchivo(char* filename);
char* crearArchivo(char* tamanio,char*nombre);
void setearValores(t_config * archivoConfig);
int verificarExistenciaDeArchivo(char* rutaArchivoConfig);
void leerArchivoDeConfiguracion( char * direccionArchivo);
void recibirComandos(int socketUMC);

#endif /* SWAP_H_ */
