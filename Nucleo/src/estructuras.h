#ifndef NUCLEO_VIEJO_SRC_ESTRUCTURAS_H_
#define NUCLEO_VIEJO_SRC_ESTRUCTURAS_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>
#include <parser/parser.h>
#include <commons/collections/dictionary.h>
#include <parser/metadata_program.h>

#define tamanoVariable 4

t_puntero t_posicion;


typedef struct reservoContexto{
	int stackPointerConservada;
	int PCConservada;
	t_puntero dondeRetonar;
}t_reservoContexto;

typedef struct direccion{
	u_int32_t pagina;
	u_int32_t offset;
}t_direccion;

typedef struct stack{
	t_size size; //cantidad de paginas del stack
	t_intructions* base; //direccion a la base
	t_intructions* stackPointer; //direccion al tope
	//base & sp se inicializan igual
   	t_dictionary* diccionarioDeVariables; // dictionary_create(); creo que asi se inicia
   	t_puntero dondeRetorno;   //adonde ubica el valor cuando vuelve de la funcion
   	int PCReservada;

}t_stack;


typedef struct{
	int PID;					//PID
	int estado;					//ESTADO DEL PROGRAMA (0:new, 1:ready, 2:ejecutando, 3:I/O, 4:bloqueado, 5:finalizado)
	t_puntero_instruccion PC;	//LA PROXIMA INSTRUCCION A EJECUTAR
	int cant_paginas;			//CANTIDAD DE PAGINAS TOTALES DEL PROGRAMA
	t_intructions* IC;			//INDICE DE CODIGO
	int cant_instrucciones;		//CANTIDAD TOTAL DE INSTRUCCIONES EN EL CODIGO
	char* IE;					//INDICE DE ETIQUETAS
	t_list* IS;					//INDICE DEL STACK
	int socketConsola;
	int entornoActual;  //contador de cuantos stacks va a haber
	t_size tamanioIE;				//TAMAÑO DEL INDICE DE ETIQUETAS
} t_PCB;



typedef struct {
	int QUANTUM;
	int QUANTUM_SLEEP;
} t_datosNucleo;

typedef struct{
	char* PUERTO_PROG;
	char* IP_UMC;
	char* PUERTO_UMC;
	char* PUERTO_CPU;
	int QUANTUM;
	int QUANTUM_SLEEP;
	int STACK_SIZE;
	char** SEM_IDS;
	char** SEM_INIT;
	char** IO_IDS;
	char** IO_SLEEP;
	char** SHARED_VARS;
}t_configuracion;

typedef struct{
	char* id;
	int valorInicial;
	t_queue* cola;
	pthread_mutex_t mutex;

}t_semaforo;

typedef struct{
	char* id;
	int sleep;
	sem_t semaforo;
	pthread_mutex_t mutex;
	t_queue* enCola;
}t_ES;

typedef struct{
	t_nombre_compartida nombre;
	t_valor_variable* valor;
}t_variableCompartida;

typedef struct{
	t_PCB* pcb;
	int sleep;
}t_solicitudES;




t_dictionary diccionarioDeEtiquetas;





AnSISOP_funciones funciones;
AnSISOP_kernel funcionesDeNucleo;

int umcSocket;
int nucleoSocket;
t_puntero t_PC;
t_PCB* PCBActual;
//t_configuracion* configuracionCPU;
t_dictionary dictionaryCommand;

typedef void (*t_action)(void*,t_size);

typedef enum {New, Ready, EnEjecucion, Bloqueado, Exit} state;

state estado;

#endif /* NUCLEO_VIEJO_SRC_ESTRUCTURAS_H_ */
