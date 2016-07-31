#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>
#include <parser/parser.h>
#include <commons/collections/dictionary.h>
#include <parser/metadata_program.h>
#include <commons/string.h>
#include <commons/collections/list.h>


#define tamanoVariable 4

bool procesoActivo;

typedef struct direccion{
	u_int32_t pagina;
	u_int32_t offset;
	t_size size;
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

typedef struct {
	int tamanoPagina;
} t_datosUMC;

t_dictionary diccionarioDeEtiquetas;

AnSISOP_funciones funciones;
AnSISOP_kernel funcionesDeNucleo;

int umcSocket;
int nucleoSocket;
t_puntero t_PC;
t_PCB* PCBActual;
t_stack* stackActual;
t_dictionary* dictionaryCommand;
t_log* logger;


typedef void (*t_action)(void*, t_size);

typedef enum {NORMAL, FALLO_MEM, STK_OVERFLOW, KILL_CPU, END_PROCESS, ENTRADA_SALIDA, WAIT} state;

state estado;

#endif /* ESTRUCTURAS_H_ */
