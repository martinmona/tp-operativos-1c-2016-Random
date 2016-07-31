#include "hiloCPU.h"

void hiloCPU(void* socket){

	int socketEstaCPU;
	int*socketEstaCPUPunt = malloc(sizeof(int));
	memcpy(socketEstaCPUPunt,socket, sizeof(int));
	socketEstaCPU=*socketEstaCPUPunt;

	//free(socket);

	void ErrorImprimirConsola(){
		log_info(logger,"ERROR ENVIO MENSAJE CONSOLA");
	}
	log_info(logger,"Hilo CPU creado");
	int salir = 1;
	char* nombreES = string_new();
	char* nombreSem = string_new();
	t_nombre_compartida nombreVarLec;
	t_nombre_compartida nombreVarEs;
	char* textoMostrar = string_new();
	t_valor_variable* valorVar;
	valorVar=malloc(sizeof(t_valor_variable));
	int iteracionesES, largoChar, valorMostrar;
	t_variableCompartida* vc = mallocCommon(sizeof(t_variableCompartida));
	vc->valor=malloc(sizeof(t_valor_variable));
	t_semaforo* semTMP = mallocCommon(sizeof(t_semaforo));
	t_ES* dispES = mallocCommon(sizeof(t_ES));
	t_PCB* PCB = mallocCommon(sizeof(t_PCB));
	t_PCB* pcbTMP= mallocCommon(sizeof(t_PCB));
	t_solicitudES* solES = mallocCommon(sizeof(t_solicitudES));
	t_envio* mensaje = mallocCommon(sizeof(t_envio));
	int i, pid;
	char** semaforochar;
	char** varchar;
	t_semaforo* sem = mallocCommon(sizeof(t_semaforo));
	log_info(logger,"CPU Esperando programa socket %d",socketEstaCPU);
	pthread_mutex_lock(&mutexColaCPUs);
	queue_push(colaCPUs,(void*)socketEstaCPUPunt);
	pthread_mutex_unlock(&mutexColaCPUs);
	sem_post(&semCPUs);
	while(salir){
		//ESPERO LA RESPUESTA
		void errorRecibir(){
				log_error(logger,"Error al recibir de CPU. Desconectando y cerrando proceso");
				if(PCB!=NULL){
					ponerEnColaExit(PCB);
					log_info(logger,"Proceso %d pasa cola exit", PCB->PID);
				}
				//close(socketEstaCPU);
				FD_CLR(socketEstaCPU, &descriptoresLecturaCPU);
				//*socketEstaCPUPunt=-1;
				pthread_exit(NULL );
			}
		//destruirPaquete(mensaje);
		mensaje =recibir(socketEstaCPU,(void*)errorRecibir);
		switch(mensaje->codigo_Operacion){				//AGREGAR MUTEX A LA COLA
		bool mismoNombreVarLec (t_variableCompartida* vcTMP){
			log_info(logger,"Variable compartida %s valor %d",vcTMP->nombre,vcTMP->valor);
			return string_equals_ignore_case(vcTMP->nombre,nombreVarLec);
		}
		bool mismoNombreVarEs (t_variableCompartida* vcTMP){
			log_info(logger,"Variable compartida %s valor %d",vcTMP->nombre,vcTMP->valor);
			return strcasecmp(vcTMP->nombre,nombreVarEs);
		}
		bool mismoPIDTMP (t_PCB* pcb){
			return pcb->PID==pcbTMP->PID;
		}
		bool mismoPID (t_PCB* pcb){
			return pcb->PID==PCB->PID;
		}
		bool mismoNombreES(t_ES* es){
			return string_equals_ignore_case(es->id,nombreES);
		}
		bool mismoNombreSem(t_semaforo* sem){
			//log_info(logger,"semid: %s nombresem: %s", sem->id,nombreSem);
			return string_equals_ignore_case(sem->id ,nombreSem);
		}
		bool mismoSocket(int sc){
			return PCB->socketConsola==sc;
		}
			case CPU_Nucleo_FinDeQuantum://QUANTUM
				log_info(logger,"Proceso %d finalizo por quantum", PCB->PID);
				PCB = deserializarPCB(mensaje->data);
				pthread_mutex_lock(&mutexListaEjecucion);
				list_remove_by_condition(listaEjecucion,(void*)mismoPID);
				pthread_mutex_unlock(&mutexListaEjecucion);
				PCB->estado = Ready;					//READY
				pthread_mutex_lock(&mutexColaListo);
				queue_push(colaListo, (void*)PCB);				//LO METO EN LA COLA
				pthread_mutex_unlock(&mutexColaListo);
				sem_post(&semListo);
				pthread_mutex_lock(&mutexColaCPUs);
				queue_push(colaCPUs,(void*)socketEstaCPUPunt);
				pthread_mutex_unlock(&mutexColaCPUs);
				sem_post(&semCPUs);
				PCB=NULL;
				break;							//LIBERO LA CPU

			case CPU_Nucleo_IO://IO
				//Recibo pcb, nombre dispositivoES y cantidad de veces que lo usa
				//Obtener pcb
				PCB = deserializarPCB(mensaje->data);
				destruirPaquete(mensaje);
				mensaje = recibir(socketEstaCPU,NULL);
				memcpy(nombreES,mensaje->data,mensaje->data_size-sizeof(int));
				largoChar = mensaje->data_size-sizeof(int);
				memcpy(&iteracionesES,mensaje->data+largoChar,sizeof(int));
				log_info(logger,"Solicitud IO. Dispositivo: %s Iteraciones: %d",nombreES,iteracionesES);
				pthread_mutex_lock(&mutexListaEjecucion);
				list_remove_by_condition(listaEjecucion,(void*)mismoPID);
				pthread_mutex_unlock(&mutexListaEjecucion);
				PCB->estado = Bloqueado;					//ENTRADA SALIDA
				//RECIBO LA E/S Y LA ANALIZO
				pthread_mutex_lock(&mutexListaDispositivosES);
				dispES = (t_ES*)list_find(listaDispositivosES, (void*)mismoNombreES);				//LO PONGO EN LA COLA DE ENTRADA SALIDA
				pthread_mutex_unlock(&mutexListaDispositivosES);
				//log_info(logger,"dispES: %s",dispES->id);
				solES->pcb = PCB;
				solES->sleep= iteracionesES;
				pthread_mutex_lock(&dispES->mutex);
				queue_push(dispES->enCola,(void*)solES);
				pthread_mutex_unlock(&dispES->mutex);
				sem_post(&dispES->semaforo);				//VERIFICAR QUE NO HACE FALTA ACTUALIZAR ELEMENTO EN LA LISTA
				log_info(logger,"Proceso %d pasa a bloqueado por IO", PCB->PID);
				pthread_mutex_lock(&mutexColaCPUs);
				queue_push(colaCPUs,(void*)socketEstaCPUPunt);
				pthread_mutex_unlock(&mutexColaCPUs);
				sem_post(&semCPUs);
				PCB=NULL;
				break;

			case CPU_Nucleo_Wait://WAIT
				memcpy(nombreSem,mensaje->data,mensaje->data_size);

				semaforochar=string_split(nombreSem,"\n");
				nombreSem=semaforochar[0];
				log_info(logger,"Solicitud Wait. Semaforo: %s", nombreSem);
				semTMP = (t_semaforo*)list_find(listaSemaforos,(void*)mismoNombreSem);
				pthread_mutex_lock(&semTMP->mutex);
				if(semTMP->valorInicial>0){
					//CPU sigue ejecutando mismo pcb, solo resto 1 al valor del semaforo
					destruirPaquete(mensaje);
					mensaje = pedirPaquete(strdup("OK"), CPU_Nucleo_WaitOK,3);
					enviar(socketEstaCPU, mensaje, NULL);
					log_info(logger,"Proceso %d sigue ejecutando luego de wait", PCB->PID);
				}else{
					//Pasa a bloqueado, envio mensaje a cpu y espero pcb
					destruirPaquete(mensaje);
					mensaje = pedirPaquete(strdup("NO"), CPU_Nucleo_WaitNoOk,3);
					enviar(socketEstaCPU, mensaje, NULL);
					destruirPaquete(mensaje);
					mensaje = recibir(socketEstaCPU,NULL);
					PCB = deserializarPCB(mensaje->data);
					pthread_mutex_lock(&mutexListaEjecucion);
					list_remove_by_condition(listaEjecucion,(void*)mismoPID);
					pthread_mutex_unlock(&mutexListaEjecucion);
					PCB->estado = Bloqueado;					//BLOQUEADO
					pthread_mutex_lock(&mutexListaBloqueados);
					list_add(listaBloqueados, (void*)PCB);				//LO PONGO EN LA COLA DE BLOQUEADOS
					pthread_mutex_unlock(&mutexListaBloqueados);
					//pthread_mutex_lock(&semTMP->mutex);
					queue_push(semTMP->cola,(void*)PCB);				//Lo pongo en la cola del semaforo
					//pthread_mutex_unlock(&semTMP->mutex);
					log_info(logger,"Proceso %d pasa a bloqueado por wait", PCB->PID);
					pthread_mutex_lock(&mutexColaCPUs);
					queue_push(colaCPUs,(void*)socketEstaCPUPunt);
					pthread_mutex_unlock(&mutexColaCPUs);
					sem_post(&semCPUs);
					PCB=NULL;
				}
				pthread_mutex_unlock(&semTMP->mutex);
				semTMP->valorInicial-=1;

				break;

			case CPU_Nucleo_FinProceso://EXIT --- falta implementar
				PCB = deserializarPCB(mensaje->data);
				pthread_mutex_lock(&mutexListaEjecucion);
				list_remove_by_condition(listaEjecucion,(void*)mismoPID);
				pthread_mutex_unlock(&mutexListaEjecucion);
				ponerEnColaExit(PCB);
				log_info(logger,"Proceso %d pasa cola exit", PCB->PID);
				pthread_mutex_lock(&mutexColaCPUs);
				queue_push(colaCPUs,(void*)socketEstaCPUPunt);
				pthread_mutex_unlock(&mutexColaCPUs);
				sem_post(&semCPUs);
				PCB=NULL;
				//salir=0;
				break;

			case CPU_Nucleo_Imprimir://IMPRIMIR VALOR
				memcpy(&valorMostrar, mensaje->data,sizeof(int));
				memcpy(&pid, mensaje->data+sizeof(int),sizeof(int));
				log_info(logger,"Envio a consola: %d a mostrar valor: %d", PIDS[pid],valorMostrar);
				//ENVIO EL RESULTADO
				if(list_any_satisfy(listaConsolasActivas,(void*)mismoSocket)){
					enviar(PCB->socketConsola,mensaje,(void*)ErrorImprimirConsola);
				}else{
					log_info(logger,"No se muestra mensaje en Consola.Esta caida");
				}
				break;

			case CPU_Nucleo_ImprimirTexto://IMPRIMIR TEXTO
				memcpy(textoMostrar, mensaje->data,mensaje->data_size-sizeof(int));
				memcpy(&pid, mensaje->data+mensaje->data_size-sizeof(int),sizeof(int));
				log_info(logger,"Envio a consola a mostrar valor: %s", textoMostrar);
				log_info(logger,"Envio a consola: %d PID: %d", PIDS[pid],pid);
				//ENVIO EL RESULTADO
				if(list_any_satisfy(listaConsolasActivas,(void*)mismoSocket)){
					enviar(PCB->socketConsola,mensaje,(void*)ErrorImprimirConsola);
				}else{
					log_info(logger,"No se muestra mensaje en Consola.Esta caida");
				}
				break;

			case CPU_Nucleo_PedirVariableCompartida://LEER VARIABLE COMPARTIDA
				nombreVarLec= mallocCommon(mensaje->data_size+1);
				memcpy(nombreVarLec, mensaje->data,mensaje->data_size);
				vc->nombre=arreglarNombreVarCompartida(nombreVarLec);
				log_info(logger,"Solicitud Leer variable compartida %s", vc->nombre);
				//vc->nombre=nombreVarLec;
				//LEO LA VARIABLE COMPARTIDA
				pthread_mutex_lock(&mutexVariablesCompartidas);
				vc->valor = (int*)dictionary_get(diccionarioVariablesCompartidas,vc->nombre);
				pthread_mutex_unlock(&mutexVariablesCompartidas);
				if(vc!=NULL){
				log_info(logger,"Variable encontrada: %s, Valor: %d", vc->nombre, *vc->valor);
				destruirPaquete(mensaje);
				//ENVIO LA VARIABLE COMPARTIDA
				mensaje=pedirPaquete(vc->valor,CPU_Nucleo_PedirVariableCompartida,sizeof(t_valor_variable));
				enviar(socketEstaCPU,mensaje,NULL);
				log_info(logger,"Valor de variable %s enviado a CPU", vc->nombre);
				}
				//free(nomreVarLec);
				break;

			case CPU_Nucleo_AsignarVariableCompartida://ESCRIBIR VARIABLE COMPARTIDA
				nombreVarEs= mallocCommon(mensaje->data_size-sizeof(int));
				memcpy(nombreVarEs, mensaje->data,mensaje->data_size-sizeof(int));
				vc->nombre=arreglarNombreVarCompartida(nombreVarEs);
				log_info(logger,"Solicitud Escribir variable compartida %s", vc->nombre);
				//vc->nombre=nombreVarEs;
				memcpy(valorVar, mensaje->data+mensaje->data_size-sizeof(int),sizeof(int));
				pthread_mutex_lock(&mutexVariablesCompartidas);
				vc->valor = (int*)dictionary_get(diccionarioVariablesCompartidas,vc->nombre);

				log_info(logger,"Variable encontrada: %s, Valor: %d", vc->nombre, *vc->valor);
				//ESCRIBO LA VARIABLE COMPARTIDA
				memcpy(vc->valor,valorVar,sizeof(int));
				pthread_mutex_unlock(&mutexVariablesCompartidas);
				log_info(logger,"Modificado el valor de variable. Variable: %s Valor: %d", vc->nombre, *vc->valor);
				//free(nombreVarEs);
				break;

			case CPU_Nucleo_Signal://SIGNAL

				memcpy(nombreSem,mensaje->data,mensaje->data_size);
				semaforochar=string_split(nombreSem,"\n");
				nombreSem=semaforochar[0];
				log_info(logger,"Solicitud Signal. Semaforo: %s", nombreSem);
				semTMP = (t_semaforo*)list_find(listaSemaforos,(void*)mismoNombreSem);
				pthread_mutex_lock(&semTMP->mutex);
				if(semTMP->valorInicial<0){
					//Hay procesos esperando por el semaforo, lo paso a lista
					pcbTMP = (t_PCB*)queue_pop(semTMP->cola);
					pthread_mutex_lock(&mutexListaBloqueados);
					list_remove_by_condition(listaBloqueados, (void*)mismoPIDTMP);				//LO SACO DE BLOQUEADO
					pthread_mutex_unlock(&mutexListaBloqueados);
					pthread_mutex_lock(&mutexColaListo);
					queue_push(colaListo,(void*)pcbTMP);
					pthread_mutex_unlock(&mutexColaListo);
					sem_post(&semListo);
					log_info(logger,"Proceso %d pasa a listo por signal", pcbTMP->PID);
				}
				pthread_mutex_unlock(&semTMP->mutex);
				semTMP->valorInicial+=1;
				break;

			case CPU_Nucleo_Desconexion:
				log_info(logger,"CPU se desconectara");
				if(mensaje->data_size>2){
					PCB = deserializarPCB(mensaje->data);
					pthread_mutex_lock(&mutexListaEjecucion);
					list_remove_by_condition(listaEjecucion,(void*)mismoPID);
					pthread_mutex_unlock(&mutexListaEjecucion);
					PCB->estado = Ready;					//READY
					pthread_mutex_lock(&mutexColaListo);
					queue_push(colaListo, (void*)PCB);				//LO METO EN LA COLA
					pthread_mutex_unlock(&mutexColaListo);
					sem_post(&semListo);
					salir=0;
					log_info(logger,"Proceso %d finalizo por desconexion", PCB->PID);
				}else{
					salir=0;

				}


				break;
			case CPU_Nucleo_Excepcion://STACKOVERFLOW
				PCB=deserializarPCB(mensaje->data);
				pthread_mutex_lock(&mutexListaEjecucion);
				list_remove_by_condition(listaEjecucion,(void*)mismoPID);
				pthread_mutex_unlock(&mutexListaEjecucion);
				PCB->estado =Exit;
				pthread_mutex_lock(&mutexColaSalida);
				queue_push(colaSalida,(void*)PCB);
				pthread_mutex_unlock(&mutexColaSalida);
				sem_post(&semExit);
				log_error(logger,"Error en la ejecucion del programa &d. Pasa a EXIT", PCB->PID);

				pthread_mutex_lock(&mutexColaCPUs);
				queue_push(colaCPUs,(void*)socketEstaCPUPunt);
				pthread_mutex_unlock(&mutexColaCPUs);
				sem_post(&semCPUs);
				PCB=NULL;
				break;
			case CPU_Nucleo_PCB:
				PCB=deserializarPCB(mensaje->data);
				break;
			case 99://Error cae cpu mal
				log_info(logger,"Se cayo la cpu");
				pthread_mutex_lock(&mutexListaEjecucion);
				list_remove_by_condition(listaEjecucion,(void*)mismoPID);
				pthread_mutex_unlock(&mutexListaEjecucion);
				ponerEnColaExit(PCB);
				log_info(logger,"Proceso %d pasa cola exit por error", PCB->PID);
				salir=0;
				break;

			default:
				salir = 0;
		}
		destruirPaquete(mensaje);


	}
	//free(PCB);
	log_info(logger,"Se cierra CPU socket: %d", socketEstaCPU);
	//close(socketEstaCPU);
	FD_CLR(socketEstaCPU, &descriptoresLecturaCPU);
	socketEstaCPU=-1;
	*socketEstaCPUPunt=-1;
	pthread_exit(NULL );
}

char* arreglarNombreVarCompartida(char* nombrevariable){
	char* nombreBien = mallocCommon(string_length(nombrevariable));
	int i=1;
	while (!dictionary_has_key(diccionarioVariablesCompartidas,nombreBien) && i <= string_length(nombrevariable)) {

			nombreBien=string_substring(nombrevariable,0,i);
			string_append(&nombreBien,"\0");
			i++;
	}
	return nombreBien;
}



