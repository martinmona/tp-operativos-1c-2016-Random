#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable identificador_variable) {
	if (estado != STK_OVERFLOW) {
	log_info(logger, "defino variable");

	pushStack(stackActual, identificador_variable, umcSocket);

	if (estado != STK_OVERFLOW) {
	t_puntero posicion;
		posicion = ultimaVariable(stackActual);
//		log_info(logger, "Defino variable %c en posicion %u",
//				identificador_variable, posicion);
		return posicion;
	}
    return -1;}
	return -1;}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable) {
	log_info(logger, "obtengoPosicionVariable %c", identificador_variable);
	char* identificador = charAString(identificador_variable);
	t_puntero *posicion = dictionary_get(stackActual->diccionarioDeVariables,
			identificador);
	//t_puntero posicion = posicionVariable(&(PCBActual->IS), identificador_variable);
	//log_info(logger, "posicion variable es %d", posicion);

	t_intructions direccion;
	direccion.start = (stackActual->base)->offset;
	direccion.offset = (stackActual->base)->start;
//	log_info(logger, "pag %d , off %d", direccion.offset, direccion.start);
	/*	t_puntero posicionBase = calculoDeDireccionAPosicion(direccion);
	 t_puntero posicionFinal = posicion - posicionBase;
	 log_info(logger, "posicion final %d", posicionFinal); */
	if (posicion != NULL) {
		return posicion;
		log_info(logger, "Obtengo para la variable %c, La posicion %u",
				identificador_variable, posicion);

		//	destruirPaquete(posicion);
	}
	return -1;
	log_info(logger, "return %d", -1);
}

t_valor_variable dereferenciar(t_puntero direccion_variable) {
	log_info(logger, "dereferenciar en posicion %d", direccion_variable);
	t_intructions direccion = calculoDePosicionADireccion(direccion_variable);

	t_envio* paquete = paquete_leerUMC(direccion.start, direccion.offset,
			tamanoVariable);
	//log_info(logger, "lei de la UMC pag %d , off %d", direccion.start,
		//	direccion.offset);
	t_envio* respuesta = enviar_y_esperar_respuesta_con_paquete(umcSocket,
			paquete, UMC_FalloMemoria, ejecutarSiSePerdioConexion,
			ejecutarSiFalloMemoria);

	destruirPaquete(paquete);
	t_valor_variable retorno = *((t_valor_variable*) respuesta->data);
	destruirPaquete(respuesta);
	return retorno;
	log_info(logger, "Para la variable %s Es la direccion %u",
			direccion_variable, retorno);
}

//copia el valor en la posicion
void asignar(t_puntero direccion_variable, t_valor_variable valor) {
	log_info(logger, "asignando direccion %d, valor %d", direccion_variable,
			valor);
	t_intructions direccion = calculoDePosicionADireccion(direccion_variable);
	t_envio* paquete = ecribirEnUMC(direccion.start, direccion.offset,
			tamanoVariable, &valor);
//	log_info(logger, "escribo en UMC pag %d off %d valor %d", direccion.start,
		//	direccion.offset, valor);
	t_envio* nuevo_paquete = enviar_y_esperar_respuesta_con_paquete(umcSocket,
			paquete, UMC_FalloMemoria, ejecutarSiSePerdioConexion,
			ejecutarSiHuboStackOverflow);

	destruirPaquete(paquete);
	destruirPaquete(nuevo_paquete);
	log_info(logger, "Para la direccion %u El valor %u", direccion_variable,
			valor);
}

void irAlLabel(t_nombre_etiqueta etiqueta) {
	char** etiquetaSinN = mallocCommon(string_length(etiqueta));
	etiquetaSinN = string_split(etiqueta, "\n");
	//printf("Etiqueta sin n: %s", etiquetaSinN[0]);
	t_puntero_instruccion nuevoPC = metadata_buscar_etiqueta(etiquetaSinN[0],
			PCBActual->IE, PCBActual->tamanioIE);

	PCBActual->PC = nuevoPC;
	log_info(logger,
			"Salto etiqueta %s  Nuevo PC: %d.  Etiquetas: %s",
			etiqueta, PCBActual->PC, PCBActual->IE);

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar) {
	log_info(logger, "voy a llamar con retorno");

	t_stack* nuevoStack;
	nuevoStack = mallocCommon(sizeof(t_stack));
	nuevoStack->base = stackActual->base;
	nuevoStack->stackPointer = stackActual->stackPointer;
	nuevoStack->size = stackActual->size;
	nuevoStack->diccionarioDeVariables = dictionary_create();
	nuevoStack->dondeRetorno = dondeRetornar; // !!
	stackActual->PCReservada = PCBActual->PC;
//	log_info(logger, "PC en stack: %d", PCBActual->PC);
	PCBActual->entornoActual++;
	list_add_in_index(PCBActual->IS, PCBActual->entornoActual,
			(void*) nuevoStack);
	stackActual = nuevoStack;

	irAlLabel(etiqueta);
//me ubico en la funcion
}

//vuelve a la funcion principal
void retornar(t_valor_variable retorno) {
	log_info(logger, "voy a retornar");
	t_puntero dondeRetornar = stackActual->dondeRetorno;

	//t_puntero* direccion_variable = popStack(stackActual, dondeRetornar,
	// umcSocket, ejecutarSiSePerdioConexion, ejecutarSiFalloMemoria);

	asignar(dondeRetornar, retorno);
	//log_info(logger, "Retorno a %u Con valor %u", direccion_variable, retorno);

	PCBActual->entornoActual--;
	stackActual = (t_stack*) list_get(PCBActual->IS, PCBActual->entornoActual);
	PCBActual->PC = stackActual->PCReservada;
//	log_info(logger, "PC  Actualizado %d  Entorno actual %d", PCBActual->PC,
//			PCBActual->entornoActual);

}

// ESTO VA CN EL NUCLEO

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {

	t_envio* respuesta = enviar_y_esperar_respuesta(nucleoSocket, variable,
			CPU_Nucleo_PedirVariableCompartida, string_length(variable) + 1,
			NoHayErrorPosible, ejecutarSiSePerdioConexion, noHacerNada);
	t_valor_variable retorno = *(t_valor_variable*) respuesta->data;
	destruirPaquete(respuesta);
	//	free(variable);
	return retorno;
	log_info(logger, "Obtengo valor de var compartida %s, Con valor %u", variable, retorno);

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,
		t_valor_variable valor) {

	t_envio* paquete = paquete_asignarCompartida(variable, valor);
	enviar(nucleoSocket, paquete, ejecutarSiSePerdioConexion);
	destruirPaquete(paquete);
	return valor;
	log_info(logger, "Asigno valor de var compartida %s, Con valor %u", variable, valor);


}

void imprimir(t_valor_variable valor_mostrar) {
	t_envio* paquete = pedirPaquete(&valor_mostrar,
	CPU_Nucleo_Imprimir, sizeof(t_valor_variable));
	aniadirAlPaquete(paquete, &PCBActual->PID, sizeof(int));
	enviar(nucleoSocket, paquete, ejecutarSiSePerdioConexion);
	destruirPaquete(paquete);
	log_info(logger, "imprimo %u", valor_mostrar);
}

void imprimirTexto(char* texto) {
	t_envio* paquete = pedirPaquete(texto, CPU_Nucleo_ImprimirTexto,
			string_length(texto) + 1);
	aniadirAlPaquete(paquete, &PCBActual->PID, sizeof(int));
	enviar(nucleoSocket, paquete, ejecutarSiSePerdioConexion);
	destruirPaquete(paquete);
	log_info(logger, "imprimo texto %u", texto);
	//free(texto);
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	t_size sizeString = string_length(dispositivo) + 1;
	//dispositivo[sizeString]="\0";
	void* datosExtra;
	datosExtra = mallocCommon(sizeString + sizeof(int));
	memcpy(datosExtra, dispositivo, sizeString);
	memcpy(datosExtra + sizeString, &tiempo, sizeof(int));
	cambiarEstado(ENTRADA_SALIDA);
	terminarProceso(datosExtra, sizeString + sizeof(int));
	free(datosExtra);
	log_info(logger, "entrada salida dispositivo %s, tardo %u", dispositivo, tiempo);

}

//operacion de kernel
void wait(t_nombre_semaforo identificador_semaforo) {
	t_envio* paquete = pedirPaquete(identificador_semaforo, CPU_Nucleo_Wait,
			string_length(identificador_semaforo) + 1);
	t_envio* respuesta = enviar_y_esperar_respuesta_con_paquete(nucleoSocket,
			paquete, NoHayErrorPosible, ejecutarSiSePerdioConexion,
			noHacerNada);
	destruirPaquete(paquete);
	if (respuesta->codigo_Operacion != CPU_Nucleo_WaitOK) {
		cambiarEstado(WAIT);
		terminarProceso(NULL, 0);
	}
	destruirPaquete(respuesta);
	log_info(logger, "wait %s", identificador_semaforo);
}

//operacion de kernel
void signal_(t_nombre_semaforo identificador_semaforo) {
	t_envio* paquete = pedirPaquete(identificador_semaforo,
	CPU_Nucleo_Signal, string_length(identificador_semaforo) + 1);
	enviar(nucleoSocket, paquete, NULL);
	destruirPaquete(paquete);
	log_info(logger, "signal %s", identificador_semaforo);
}
//tengo que cambiar nombre de signal porque ya existe otro signal

void finalizar() {
	cambiarEstado(END_PROCESS);
	terminarProceso(NULL, 0);
log_info(logger, "finalizo");
}
