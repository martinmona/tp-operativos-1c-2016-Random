#ifndef STACK_H_
#define STACK_H_

#include "funcionesCPU.h"
#include "estructuras.h"

void pushStack(t_stack* stack, t_nombre_variable dato, int umcSocket);
t_puntero* popStack(t_stack* stack, t_puntero direccion_variable, int umcSocket,
		void (*ejecutarSiSePerdioConexion)(), void (*ejecutarSiFalloMemoria)());
t_puntero posicionVariable(t_stack* stack, t_nombre_variable identificador_variable);
t_puntero ultimaVariable(t_stack* stack);
t_intructions direccionFuncion (t_stack* stack, t_puntero direccion_variable);
t_puntero calculoStackOverFlow(int paginaInicialStack, int cantidadPaginas);

#endif /* STACK_H_ */
