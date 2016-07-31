/*
 * FuncionesUMC.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESUMC_H_
#define FUNCIONESUMC_H_

#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <commons/string.h>
#include <commons/config.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include "common_sockets.h"
#include "conexiones.h"

//Estructuras --------------------------------------
/*
 * INFORMACION
 *
 * -Manejo una lista de marcos, para determinar direccion donde comienzan, y si estan asignados
 * -Hay Lista para tablas de paginas de procesos, un item por proceso.
 * -Para lograr el reemplazo local de paginas, hay otra lista de un item por proceso
 * 	que tiene un puntero y una lista de paginas de ese proceso en memoria, con datos de clock
 * 	para que el puntero avance y vea cual se reemplazara
 * -Es necesario actualizar la lista de reemplazos y la tabla de paginas del proceso al
 * Agregar, eliminar y/o modificar paginas
 *
 */
//Estructura que almacena los datos del archivo de configuracion
typedef struct{
	char* puertoEscucha;
	char* ipSWAP;
	char* puertoSWAP;
	int cantidadMarcos;
	int tamanioMarco;
	int maximoMarcoXProceso;
	int algoritmoPlanificacion;	//0=Clock , 1= Clock Modificado
	int entradasTLB;				//0 no hay TLB
	int retardoMemoria;
	bool usoTLB;

} configuracion;

typedef struct{
	int numeroPagina;
	bool presencia;
	bool modificacion;
	int frame;
}t_entradaPagina;

typedef struct{
	int PID;
	int marcosAsignados;
	t_list* paginas; //Lista de t_entradaPagina
}t_tablaPaginas;

typedef struct{
	int id;
	char* comienzo;
	bool asignado;
}t_marco;

typedef struct{
	int pid;
	int pagina;
	char* direccion;
}t_entradaTLB;

typedef struct{
	int pid;
	t_list* datosxPaginas; //Lista de t_datosxPaginas
	int puntero;
}t_datosCargaMemoria;

typedef struct{
	int idPag;
	bool clock;
	bool clockModificado;
}t_datosxPaginas;

//------------------------------------------------

//Variables---------------------------------------


configuracion miConfig;
t_log* logger;
char* memoria;
pthread_mutex_t semaforoGlobal;
pthread_mutex_t semaforoSWAP;
t_list* listaPaginasMemoria; //Lista de t_datosCargaMemoria con cada pagina en memoria
t_list* listaMarcos;	//Lista con los marcos
t_list* listaTLB;
t_list* listaTablasdePaginas; //Lista con las tablas de pagina de cada proceso
//-------------------------------------------------

t_entradaPagina* obtenerEntradaDePagina(t_tablaPaginas* tp, int id);
void agregarEntradaEnTLB(t_entradaTLB* entrada);
void escribirPagina(t_tablaPaginas* tablaProceso,t_datosCargaMemoria* datosCM, int idPagina, int offset, int tamanio, void*data, char* dirPagPLE);
void cargarArchivoConfig();
char* obtenerDireccionDePagina(t_tablaPaginas* tp, int idP, char* dirPagPLE);
void inicializar();
void hiloCPU(void* socket);
void hiloKernel(void* socket);

#endif /* FUNCIONESUMC_H_ */
