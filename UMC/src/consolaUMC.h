/*
 * consolaUMC.h
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#ifndef CONSOLAUMC_H_
#define CONSOLAUMC_H_

#include <ctype.h>
#include "commons/string.h"
#include <pthread.h>
#include <stdlib.h>

enum Commands
{
	// esto define los comandos en la forma en que los vamos a manipular internamente
	// son enteros que representan a un estado, que no sean solo números ayuda a la lectura
	// pero es solo de uso interno
	// si se ingresa un nuevo comando o se modifica, se debe modificar también su representación
	// del lado del usuario, en la lista de arriba.
	ayuda = 0,
	limpiar,
	retardos,
	dump,
	flush,
	salir,
	enter
};


void consolaUMC ();

int leeComando(void);

int orden;

#endif /* CONSOLAUMC_H_ */
