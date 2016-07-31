#include "funcionesCPU.h"

AnSISOP_funciones funciones = { .AnSISOP_definirVariable=definirVariable,
.AnSISOP_obtenerPosicionVariable=obtenerPosicionVariable,
.AnSISOP_dereferenciar = dereferenciar, .AnSISOP_asignar = asignar,
.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
.AnSISOP_asignarValorCompartida = asignarValorCompartida,
.AnSISOP_irAlLabel = irAlLabel,
.AnSISOP_llamarConRetorno = llamarConRetorno,
.AnSISOP_retornar = retornar,
.AnSISOP_imprimir = imprimir,
.AnSISOP_imprimirTexto = imprimirTexto,
.AnSISOP_entradaSalida = entradaSalida,
.AnSISOP_finalizar = finalizar};

AnSISOP_kernel funcionesDeNucleo = {
.AnSISOP_wait = wait,
.AnSISOP_signal = signal_ };

//Variables Globales -------------------------------------------




//Fin Variables Globales ------------------------------------

void inicializarLog(){
	logger = log_create("../resources/log.txt", "CPU", true, LOG_LEVEL_DEBUG);
	log_info(logger, "Log creado");
}

void cargarSenales() {
		signal(SIGUSR1, senalDesconexion);
log_info(logger, "cargo señales");}

void crearConfiguracion(char* configPath) {
	configCPU = config_create(configPath);
//	log_info(logger,"El archivo de configuracion anda");
}



t_PCB* obtenerPCB() {
	t_PCB* nuevo = mallocCommon(sizeof(t_PCB));

	//nuevo = mallocCommon(sizeof(t_PCB));
	//t_envio* recibirPCB =mallocCommon(sizeof(t_envio));
	t_envio* recibirPCB= recibir(nucleoSocket, NULL);
	if(recibirPCB->codigo_Operacion != CPU_Nucleo_ProgramaAEjecutar) {
		log_info(logger,"Fallo entrega PCB");
		estado = END_PROCESS;
		procesoActivo=false;
		//return NULL ;
	}
	memcpy(&quantum,recibirPCB->data,sizeof(int));
	memcpy(&QUANTUM_SLEEP,recibirPCB->data+sizeof(int),sizeof(int));
	recibirPCB= recibir(nucleoSocket, NULL);
	log_info(logger,"Desempaquetando");
	nuevo = desempaquetarPCB(recibirPCB->data);
	destruirPaquete(recibirPCB);
	log_info(logger,"PCB OK");
	recibirPCB=empaquetarPCB(nuevo, CPU_Nucleo_PCB);
	enviar(nucleoSocket,recibirPCB,NULL);
	//log_info(logger, "PCB recibida correctamente. Empezando a ejecutar el proceso %d",nuevo->PID);
	return nuevo;
}


void ejecutarInstruccion(int QUANTUM_SLEEP) {

//con PC saco indice de codigo de el pcb
	t_intructions calculoIndice =  PCBActual->IC[PCBActual->PC];
	uint32_t contador = 0;
		while (calculoIndice.start >= (tamanoPagina + (tamanoPagina * contador))) {
			contador++;
		}
		uint32_t paginaAPedir = contador;
		t_puntero start = calculoIndice.start - (paginaAPedir*tamanoPagina);
	log_info(logger,"Solicito pagina: %d start: %d offset: %d",paginaAPedir, start, calculoIndice.offset);
		t_envio* instruccion = solicitar_algo_UMC(paginaAPedir, start, calculoIndice.offset);

	char* INSTRUCCION = mallocCommon(instruccion->data_size+1);

	memcpy(INSTRUCCION,instruccion->data,instruccion->data_size);
//	log_info(logger,"Tamano instruccion %d", instruccion->data_size);
	INSTRUCCION[instruccion->data_size] = '\0';
	log_info(logger, "Instruction encontrada: %s", INSTRUCCION);

//aumento PC
	(PCBActual->PC)++;
//parseo
	analizadorLinea((char * const ) INSTRUCCION, &funciones, &funcionesDeNucleo);

	free(INSTRUCCION);
	destruirPaquete(instruccion);
	sleep(QUANTUM_SLEEP/1000);
	log_info(logger, "aplicando QUANTUM_SLEEP %d ms",QUANTUM_SLEEP);
}

void avisarProcesoActivo() {
	procesoActivo = true;
	t_envio* paquete = pedirPaquete(&(PCBActual->PID),
			CPU_UMC_ProcesoActivo, sizeof(t_puntero));
	enviar(umcSocket, paquete, NULL );
	destruirPaquete(paquete);
	paquete = recibir(umcSocket, NULL );
	if (paquete->codigo_Operacion != CPU_UMC_ProcesoActivo) {
		log_error(logger, "No se pudo avisar el proceso activo");
		estado = END_PROCESS;
	}
	destruirPaquete(paquete);
}


t_datosNucleo ConectarConNucleo(){
	char* IP_NUCLEO = config_get_string_value(configCPU, "IP_NUCLEO");
	char* PUERTO_NUCLEO = config_get_string_value(configCPU, "PUERTO_NUCLEO");
	log_info(logger,"IP_NUCLEO: %s", IP_NUCLEO);
	log_info(logger,"PUERTO_NUCLEO: %s", PUERTO_NUCLEO);
	nucleoSocket = conectar(IP_NUCLEO, PUERTO_NUCLEO);
	if (nucleoSocket == -1) {
		log_error(logger, "Fallo la conexion al nucleo");
		destruirConexion();
	}
		return handshakeNucleo();
}

t_datosNucleo handshakeNucleo() {
	int pidCPU = getpid();
	t_envio* paquete = pedirPaquete(&pidCPU, CPU_Nucleo_Handshake, sizeof(int));
	enviar(nucleoSocket, paquete, destruirConexion);
	destruirPaquete(paquete);
	paquete = recibir(nucleoSocket, NULL );
	if (paquete->codigo_Operacion != CPU_Nucleo_Ok) {
		log_error(logger, "Fallo handshake con nucleo");
		destruirPaquete(paquete);
		destruirConexion();
	}
	log_info(logger, "Exito en el handshake con nucleo");

	t_datosNucleo nucleoData = obtenerDataNuleo(paquete->data);
	destruirPaquete(paquete);
	return nucleoData;
}

t_datosNucleo obtenerDataNuleo(void* data) {
	t_datosNucleo nucleoData;
	/*memcpy(&nucleoData, data, sizeof(uint32_t));
	nucleoData.QUANTUM = *(uint32_t*) (data);
	nucleoData.QUANTUM_SLEEP = *(uint32_t*) (data + sizeof(uint32_t));

	*/
	memcpy(&nucleoData.QUANTUM,data,sizeof(int));
	memcpy(&nucleoData.QUANTUM_SLEEP,data+sizeof(int),sizeof(int));
	return nucleoData;
}

t_datosUMC conectarConUMC(){
	char* IP_UMC = config_get_string_value(configCPU, "IP_UMC");
	log_info(logger,"IP_UMC: %s", IP_UMC);
	char* PUERTO_UMC = config_get_string_value(configCPU, "PUERTO_UMC");
	log_info(logger,"PUERTO_UMC: %s", PUERTO_UMC);
	umcSocket = conectar(IP_UMC, PUERTO_UMC);
	if (umcSocket == -1) {
		log_error(logger, "Fallo la conexion a UMC");
		destruirConexion();
	}
	log_info(logger, "Conexion con UMC esta lista, preparando handshake");
	return handshakeUMC();
}

t_datosUMC handshakeUMC() {
	t_envio* paquete = pedirPaquete("a", CPU_UMC_Handshake, 2);
	enviar(umcSocket, paquete, destruirConexion);
	destruirPaquete(paquete);
	paquete = recibir(umcSocket, NULL );
	if (paquete->codigo_Operacion != CPU_UMC_Ok) {
		log_error(logger,"Fallo handshake con UMC");
		destruirPaquete(paquete);
		destruirConexion();
	}
	log_info(logger,"Exito en handshake con UMC");
	t_datosUMC UMCData = obtenerPagina(paquete->data);
	destruirPaquete(paquete);
	return UMCData;
}

void destruirConexion() {
	close(umcSocket);
	close(nucleoSocket);
	log_destroy(logger);
	config_destroy(configCPU);
	exit(EXIT_FAILURE);
}

t_datosUMC obtenerPagina(void* data) {
	t_datosUMC UMCData;
	memcpy(&UMCData, data, sizeof(uint32_t));
	UMCData.tamanoPagina = *(uint32_t*) (data);
	return UMCData;
}

void terminarProceso(void* datosExtra, t_size sizeExtra) {
	char* translate = string_itoa(estado);
	t_action accion = dictionary_get(dictionaryCommand,translate);
	accion(datosExtra,sizeExtra);
	procesoActivo = false;
	free(translate);
}
typedef void (*t_action)(void*, t_size);


#define command(state,function) dictionary_put(dictionaryCommand,string_itoa(state),function);

void acciones() {
	dictionaryCommand = dictionary_create();
	log_info(logger,"creo acciones");
	command(STK_OVERFLOW, terminaExcepcion)
	command(FALLO_MEM, terminaExcepcion)
	command(WAIT, terminaBloqueoDeSemaforos)
	command(ENTRADA_SALIDA, terminaBloqueoIO)
	command(NORMAL, terminaFinQuantum)
	command(KILL_CPU, terminarCPU)
	command(END_PROCESS, terminaProcesoFinalizado)
}

t_puntero calculoDeDireccionAPosicion(t_intructions direccion){
	if(direccion.start == 0){
		return (direccion.start + direccion.offset);
	}
	return ((direccion.start * tamanoPagina) + direccion.offset);
}

t_intructions calculoDePosicionADireccion(t_puntero posicion){
	t_intructions direccion;
	int resultado = (posicion / tamanoPagina);
	int resto = (posicion % tamanoPagina);
	if(resultado == 0){
		direccion.start = 0;
		direccion.offset = posicion;
		return direccion;
	}
	direccion.start = resultado;
	direccion.offset = resto;
	return direccion;
}

void ejecutarSiHuboStackOverflow() {
	char* mensaje = strdup("STACK OVERFLOW");
	cambiarEstado(STK_OVERFLOW);
	terminarProceso(mensaje,strlen(mensaje) + 1);
	free(mensaje);
}

void ejecutarSiFalloMemoria() {
	char* mensaje = strdup("FALLO DE MEMORIA");
	cambiarEstado(FALLO_MEM);
	terminarProceso(mensaje,strlen(mensaje) + 1);
	free(mensaje);
}

bool estadoDesconexion(state estado) {
	return (estado == KILL_CPU);
}

void cambiarEstado(state toState){
	if (!estadoDesconexion(estado)) {
		estado = toState;
	}
}

void ejecutarSiSePerdioConexion() {

}

void noHacerNada(){

}

void senalDesconexion(int signal) {
	if (procesoActivo == true) {
		estado = KILL_CPU;
	} else {
		terminarCPUSinPCB(NULL, 0);
	}
}



char* charAString(char element){
	char* new = mallocCommon(2);
	*new = element;
	*(new + 1) = '\0';
	return new;
}

t_PCB* desempaquetarPCB(void* data){
	t_PCB* pcbTmp = mallocCommon(sizeof(t_PCB));

		int tamChar, tamDic, i, tamLista,k;
		char* texto = string_new();
		t_puntero valor;
		int tamAnterior = 0;
		memcpy(&pcbTmp->PID, data, sizeof(int));
		tamAnterior+=sizeof(int);
		memcpy(&pcbTmp->estado, data+tamAnterior, sizeof(int));
		tamAnterior+=sizeof(int);
		memcpy(&pcbTmp->PC, data+tamAnterior, sizeof(t_puntero_instruccion));
		tamAnterior+=sizeof(t_puntero_instruccion);
		memcpy(&pcbTmp->cant_instrucciones, data+tamAnterior, sizeof(int));
		tamAnterior+=sizeof(int);
		memcpy(&pcbTmp->cant_paginas, data+tamAnterior, sizeof(int));
		tamAnterior+=sizeof(int);
		int j;
		t_intructions* intTmp=mallocCommon(sizeof(t_intructions)*pcbTmp->cant_instrucciones);
		for (j = 0; j < pcbTmp->cant_instrucciones; ++j) {
			t_intructions* instr =mallocCommon(sizeof(t_intructions));
			memcpy(instr, data+tamAnterior, sizeof(t_intructions));
			tamAnterior+=sizeof(t_intructions);
			intTmp[j].start=instr->start;
			intTmp[j].offset=instr->offset;
		}
		pcbTmp->IC=intTmp;
		memcpy(&pcbTmp->tamanioIE, data+tamAnterior, sizeof(t_size));
		tamAnterior+=sizeof(t_size);
		pcbTmp->IE = malloc(pcbTmp->tamanioIE);
		memcpy(pcbTmp->IE, data+tamAnterior, pcbTmp->tamanioIE);
		tamAnterior+=pcbTmp->tamanioIE;
		memcpy(&tamLista,data+tamAnterior,sizeof(int));
		tamAnterior+=sizeof(int);
		pcbTmp->IS=list_create();
		for (k = 0; k < tamLista; ++k) {
			t_stack* stackTmp = mallocCommon(sizeof(t_stack));
			stackTmp->base=mallocCommon(sizeof(t_intructions));
			stackTmp->stackPointer=mallocCommon(sizeof(t_intructions));
			stackTmp->diccionarioDeVariables = dictionary_create();
			memcpy(&stackTmp->stackPointer->start, data+tamAnterior, sizeof(t_puntero_instruccion));
			tamAnterior+=sizeof(t_puntero_instruccion);
			memcpy(&stackTmp->stackPointer->offset, data+tamAnterior, sizeof(t_size));
			tamAnterior+=sizeof(t_size);
			memcpy(&stackTmp->size, data+tamAnterior, sizeof(t_size));
			tamAnterior+=sizeof(t_size);
			memcpy(&stackTmp->base->start, data+tamAnterior, sizeof(t_puntero_instruccion));
			tamAnterior+=sizeof(t_puntero_instruccion);
			memcpy(&stackTmp->base->offset, data+tamAnterior, sizeof(t_size));
			tamAnterior+=sizeof(t_size);
			memcpy(&stackTmp->dondeRetorno, data+tamAnterior, sizeof(int));
			tamAnterior+=sizeof(int);
			memcpy(&stackTmp->PCReservada, data+tamAnterior, sizeof(int));
			tamAnterior+=sizeof(int);
			memcpy(&tamDic, data+tamAnterior, sizeof(int));
			tamAnterior+=sizeof(int);
			for (i = 0; i < tamDic; ++i) {
				memcpy(&tamChar, data+tamAnterior, sizeof(int));
				tamAnterior+=sizeof(int);
				memcpy(texto, data+tamAnterior, tamChar);
				tamAnterior+=tamChar;
				memcpy(&valor, data+tamAnterior, sizeof(t_puntero));
				tamAnterior+=sizeof(t_puntero);
				dictionary_put(stackTmp->diccionarioDeVariables,texto,valor);
				//log_info(logger,"tamaño diccionario: %d largokey: %d key: %s valor: %d",tamDic,tamChar,texto,valor);

			}
			list_add(pcbTmp->IS,(void*)stackTmp);
		}

		memcpy(&pcbTmp->socketConsola, data+tamAnterior, sizeof(int));
		tamAnterior+=sizeof(int);
		memcpy(&pcbTmp->entornoActual, data+tamAnterior, sizeof(int));
		tamAnterior+=sizeof(int);


		return pcbTmp;
}
