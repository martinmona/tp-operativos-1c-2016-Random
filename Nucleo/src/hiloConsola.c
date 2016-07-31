#include "hiloConsola.h"


int reservarMemoria(t_PCB* PCB, char* codigo, int tamCodigo){  //RESERVA LA MEMORIA EN SWAP
	pthread_mutex_lock(&mutexEnvioUMC);
	t_envio* mensaje = mallocCommon(sizeof(t_envio));
	mensaje = pedirPaquete(&PCB->PID,RESERVAR_MEMORIA,sizeof(int));
	aniadirAlPaquete(mensaje,&PCB->cant_paginas,sizeof(int));
	aniadirAlPaquete(mensaje,codigo,tamCodigo); //FIJARSE QUE UMC RECIBE TODO EL CODIGO DE UNA. CODIGO+STACK
	//aniadirAlPaquete(mensaje,PCB.IS,sizeof(t_stack));
	enviar(socketUMC,mensaje,NULL);
	destruirPaquete(mensaje);
	mensaje = recibir(socketUMC,NULL);
	pthread_mutex_unlock(&mutexEnvioUMC);
	if(mensaje->codigo_Operacion == PROGRAMA_CARGADO_SWAP){
		destruirPaquete(mensaje);
		return 1; //RESERVO BIEN;
	}else if(mensaje->codigo_Operacion == FALTA_ESPACIO_SWAP){
		destruirPaquete(mensaje);
		return 0; //NO RESERVO;
	}
	return -1;
}
int analizarPeticionConsola(t_envio* mensaje){

	switch(mensaje->codigo_Operacion){
	case FIN_PROGRAMA_CONSOLA:

		return 0;

		break;
	default:
		log_error(logger,"Mensaje de Consola CODOP: %d no definido.", mensaje->codigo_Operacion);
	}

}

int cantidadPaginas(int tamanioArchivo){
	int resultado = tamanioArchivo / tamanioPaginas;
	if((resultado * tamanioPaginas - tamanioArchivo) == 0){
		return resultado + config.STACK_SIZE;
	}else{
		return resultado + config.STACK_SIZE + 1;
	}
}

void hiloConsola(void* socket){
	int socketEstaConsola;
	int *socketEstaConsolaPunt = malloc(sizeof(int));

	memcpy(socketEstaConsolaPunt,socket, sizeof(int));
	socketEstaConsola=*socketEstaConsolaPunt;
	log_info(logger,"Hilo Consola creado");


	int salir = 1;
	//SI ES UN NUEVO PROCESO RECIBO EL CODIGO

	t_PCB* PCB;
	t_envio* mensaje;
	mensaje = recibir(socketEstaConsola,NULL);
	int tamanio = (mensaje->data_size);
	char* codigo = mallocCommon(tamanio);
	memcpy(codigo,mensaje->data,tamanio);
	log_info(logger,"tamanio data: %d", tamanio);
	log_info(logger,"Creando nueva PCB");
	pthread_mutex_lock(&mutexPCB);
	PCB = crearPCB(codigo,tamanio,socketEstaConsola); //CREO EL PCB
	pthread_mutex_unlock(&mutexPCB);
	PIDS[PCB->PID]=socketEstaConsola;
	//PCB->IE= string_new();
	//PCB->socketConsola = socketEstaConsola;
	log_info(logger,"PCB PID: %d Creada de la consola: %d.", PCB->PID, PIDS[PCB->PID]);
	log_info(logger,"tamanio IE: %d, Contenido IE: \"%s\"",PCB->tamanioIE,PCB->IE);
	if(reservarMemoria(PCB, codigo, tamanio)){	//RESERVO LA MEMORIA EN SWAP
		PCB->estado=Ready;
		pthread_mutex_lock(&mutexColaListo);
		queue_push(colaListo,(void*)PCB);			//LO PONGO EN LA COLA DE LISTO
		log_debug(logger, "Proceso %d pasa a la cola READY", PCB->PID);
		pthread_mutex_unlock(&mutexColaListo);
		//agregar uno al productor consumidor
		destruirPaquete(mensaje);
		mensaje = pedirPaquete(strdup("OK"),PROGRAMA_CARGADO_SWAP,3);
		enviar(socketEstaConsola,mensaje,NULL);	//AVISO A LA CONSOLA QUE SE PUDO CREAR   ******CAMBIARLO POR LAS CONEXIONES NUEVAS*****
		pthread_mutex_lock(&mutexListaConsolas);
		list_add(listaConsolasActivas,(void*)socketEstaConsola);
		pthread_mutex_unlock(&mutexListaConsolas);
		sem_post(&semListo);



	}else{
		PCB->estado=Exit; //seria exit o fallo?
		destruirPaquete(mensaje);
		mensaje = pedirPaquete(strdup("FE"),FALTA_ESPACIO_SWAP,3); //FALTA_ESPACIO_SWAP hay que asignarle un codigo que entienda la consola como error
		enviar(socketEstaConsola,mensaje,NULL);//send(socket,0,sizeof(int),0);	//RECHAZO X FALTA DE ESPACIO             ******CAMBIARLO POR LAS CONEXIONES NUEVAS*****
		log_error(logger,"No hay lugar para proceso PID: %d en SWAP", PCB->PID);
		close(socketEstaConsola); 					//CIERRO LA CONEXION                     ******CAMBIARLO POR LAS CONEXIONES NUEVAS*****

		pthread_exit(NULL);
	}
	//FALTA VER COMO MANEJAR QUE RECIBA MENSAJE DE SALIR DESDE CONSOLA Y ADEMAS LE ENVIE LO QUE HACE FALTA MOSTRAR

	destruirPaquete(mensaje);
	//PARA PROCESOS YA CREADOS
	void caeConsola(){
		log_error(logger,"Se desconecto la consola. Se cerrará el proceso: %d", PCB->PID);
		bool sacoSocket(int sc){
			return socketEstaConsola==sc;
		}
		list_remove_by_condition(listaConsolasActivas,(void*)sacoSocket);

		//PCB->socketConsola=-1;

		//ponerEnColaExit(PCB);
		log_info(logger,"Proceso %d pasa cola exit", PCB->PID);
		//close(socketEstaConsola);
		FD_CLR(socketEstaConsola, &descriptoresLecturaConsola);
		log_debug(logger,"Se cerro conexion consola socket: %d", socketEstaConsola);
		pthread_exit(NULL );
	}

	while(salir){
		mensaje=recibir(socketEstaConsola,(void*)caeConsola);	//RECIBO LA INSTRUCCION					 ******CAMBIARLO POR LAS CONEXIONES NUEVAS*****
		salir = analizarPeticionConsola(mensaje);	//ANALIZO EL PEDIDO DE LA CONSOLA
		destruirPaquete(mensaje);
	}
	pthread_exit(NULL);
}

t_PCB* crearPCB(char* codigo, int tamanioArchivo, int sConsola){
	t_PCB* PCBAUX = mallocCommon(sizeof(t_PCB));
	t_stack* stack = mallocCommon(sizeof(t_stack));
	t_metadata_program* programa = metadata_desde_literal(codigo);
	//PCBAUX->IE= string_new();
	//log_info(logger,"tamanio IE %d",string_length(PCBAUX->IE));

	//string_append(&PCB->IE,"\0");
	pthread_mutex_lock(&mutexVariablesGlobales);
	PCBAUX->PID=idPCB++;
	//idPCB++;
	pthread_mutex_unlock(&mutexVariablesGlobales);
	PCBAUX->estado=New;
	PCBAUX->PC=programa->instruccion_inicio;
	/*t_intructions* instrucciones = programa->instrucciones_serializado;
	for(int i= 0; i<programa->instrucciones_size; i++){
		PCB.IC[i].start = instrucciones[i].start;
		PCB.IC[i].offset = instrucciones[i].offset;
	}*/
	PCBAUX->IC=programa->instrucciones_serializado;
	PCBAUX->cant_instrucciones = programa->instrucciones_size;
	PCBAUX->IE= mallocCommon(programa->etiquetas_size);
	PCBAUX->tamanioIE = programa->etiquetas_size;
		memcpy(PCBAUX->IE,programa->etiquetas,programa->etiquetas_size);
	PCBAUX->cant_paginas=cantidadPaginas(tamanioArchivo);
	//PCB.IS.[0].start = PCB.cant_paginas - config.STACK_SIZE;
	//PCB.IS[0].offset = 0;
	stack->base = mallocCommon(sizeof(t_intructions));
	stack->stackPointer = mallocCommon(sizeof(t_intructions));
	stack->base->start = PCBAUX->cant_paginas - config.STACK_SIZE; //copio calculo arriba
	stack->base->offset = 0;
	stack->stackPointer->start = PCBAUX->cant_paginas - config.STACK_SIZE; //copio calculo arriba
	stack->stackPointer->offset = 0;
	stack->diccionarioDeVariables = dictionary_create();
	stack->size = config.STACK_SIZE;
	stack->PCReservada=0;
	stack->dondeRetorno=0;
	PCBAUX->IS=list_create();
	list_add(PCBAUX->IS,(void*)stack);
	PCBAUX->entornoActual=0;
	PCBAUX->socketConsola=sConsola;
	//PCBAUX->consolaCaida=false;
	return PCBAUX;

}



