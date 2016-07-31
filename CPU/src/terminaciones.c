#include "terminaciones.h"

void terminaFinQuantum(void* dataExtra, t_size extraSize) {
	t_envio* paquete = empaquetarPCB(PCBActual, CPU_Nucleo_FinDeQuantum);
	log_info(logger, "Proceso termino quantum");
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaExcepcion(void* dataExtra, t_size extraSize) {
	char* error_msg = (char*) dataExtra;
	t_envio* paquete = empaquetarPCB(PCBActual, CPU_Nucleo_Excepcion);
	log_info(logger,"Proceso termino con excepcion: %s", error_msg);
	//aniadirAlPaquete(paquete, error_msg, extraSize);
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaBloqueoIO(void* dataExtra, t_size extraSize) {
	t_envio* paquete = empaquetarPCB(PCBActual, CPU_Nucleo_IO);
	//aniadirAlPaquete(paquete, dataExtra, extraSize);
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
	paquete = pedirPaquete(dataExtra, CPU_Nucleo_IO, extraSize);
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaBloqueoDeSemaforos(void* dataExtra, t_size extraSize) {
	t_envio* paquete = empaquetarPCB(PCBActual, CPU_Nucleo_ProcesoEnEspera);
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminaProcesoFinalizado(void* dataExtra, t_size extraSize) {
	t_envio* paquete = empaquetarPCB(PCBActual, CPU_Nucleo_FinProceso);
	log_info(logger,"Proceso %d finalizo ejecucion", PCBActual->PID);
	enviar(nucleoSocket, paquete, NULL );
	destruirPaquete(paquete);
}

void terminarCPUSinPCB() {
	t_envio* info1 = pedirPaquete("a", CPU_Nucleo_Desconexion, 2);
	t_envio* info = pedirPaquete("a", CPU_UMC_Cae, 2);
	enviar(umcSocket, info, NULL );
	enviar(nucleoSocket, info1, NULL );
	destruirPaquete(info1);
	destruirPaquete(info);
	close(umcSocket);
	close(nucleoSocket);
	config_destroy(configCPU);
	exit(0);
}

void terminarCPU(void* dataExtra, t_size size) {
	if (procesoActivo) {
		t_envio* pcbAEnviar = empaquetarPCB(PCBActual, CPU_Nucleo_Desconexion);
		enviar(nucleoSocket, pcbAEnviar, NULL );
		destruirPaquete(pcbAEnviar);
	} else {
        t_envio* cerrar = pedirPaquete("a",CPU_Nucleo_Desconexion,2);
        enviar(nucleoSocket,cerrar,NULL);
        destruirPaquete(cerrar);
        }
           t_envio* info = pedirPaquete("a", CPU_UMC_Cae, 2);
           enviar(umcSocket, info, NULL );
           destruirPaquete(info);
           close(umcSocket);
	       close(nucleoSocket);
	       free(PCBActual);
	       dictionary_destroy(stackActual->diccionarioDeVariables);
	       config_destroy(configCPU);
	       exit(0);

}

t_envio* empaquetarPCB(t_PCB* pcbTmp, int codOp){
	t_envio* mensaje= mallocCommon(sizeof(t_envio));
		int tamIE, tamDic, tamChar,tamStacks;
		mensaje = pedirPaquete(&pcbTmp->PID,codOp,sizeof(int));
		aniadirAlPaquete(mensaje,&pcbTmp->estado,sizeof(int));
		aniadirAlPaquete(mensaje,&pcbTmp->PC,sizeof(t_puntero_instruccion));
		aniadirAlPaquete(mensaje,&pcbTmp->cant_instrucciones,sizeof(int));
		aniadirAlPaquete(mensaje,&pcbTmp->cant_paginas,sizeof(int));
		//aniadirAlPaquete(mensaje,&pcbTmp->cant_instrucciones,sizeof(int));
		int i;
		for(i=0;pcbTmp->cant_instrucciones > i;i++){
			aniadirAlPaquete(mensaje,&pcbTmp->IC[i],sizeof(t_intructions));
		}
		//aniadirAlPaquete(mensaje,pcbTmp->IC,sizeof(t_intructions));


		aniadirAlPaquete(mensaje,&pcbTmp->tamanioIE,sizeof(t_size));
		aniadirAlPaquete(mensaje,pcbTmp->IE,pcbTmp->tamanioIE);
		tamStacks=list_size(pcbTmp->IS);
		aniadirAlPaquete(mensaje,&tamStacks,sizeof(int));
		void enviarDatosLista(t_stack* stack){
			aniadirAlPaquete(mensaje,&stack->stackPointer->start,sizeof(t_puntero_instruccion));
			aniadirAlPaquete(mensaje,&stack->stackPointer->offset,sizeof(t_size));
			aniadirAlPaquete(mensaje,&stack->size,sizeof(t_size));
			aniadirAlPaquete(mensaje,&stack->base->start,sizeof(t_puntero_instruccion));
			aniadirAlPaquete(mensaje,&stack->base->offset,sizeof(t_size));
			aniadirAlPaquete(mensaje,&stack->dondeRetorno,sizeof(int));
			aniadirAlPaquete(mensaje,&stack->PCReservada,sizeof(t_size));
			tamDic = dictionary_size(stack->diccionarioDeVariables);
			aniadirAlPaquete(mensaje, &tamDic,sizeof(int));
			void enviarDatosDic(char* key, t_puntero valor){
				tamChar = string_length(key);
				tamChar++;
				aniadirAlPaquete(mensaje,&tamChar,sizeof(int));
				aniadirAlPaquete(mensaje,key,tamChar);
				aniadirAlPaquete(mensaje,&valor,sizeof(t_puntero));
				//log_info(logger,"tamaño diccionario: %d largokey: %d key: %s valor: %d",tamDic,tamChar,key,valor);
			}
			dictionary_iterator(stack->diccionarioDeVariables,(void*)enviarDatosDic);
		}
		list_iterate(pcbTmp->IS,(void*)enviarDatosLista);

		aniadirAlPaquete(mensaje,&pcbTmp->socketConsola,sizeof(int));
		aniadirAlPaquete(mensaje,&pcbTmp->entornoActual,sizeof(int));

		return mensaje;
}
