#include "FuncionesUMC.h"

void aplicarRetardo(char* texto) {
	//Demoro el retardo configurado en milisegundos
	sleep(miConfig.retardoMemoria/1000);
	log_info(logger, "Retardo por lectura en memoria de: %d ms. Comentario: %s", miConfig.retardoMemoria, texto);
}
void inicializarLog(){
	logger = log_create("../resources/log.txt", "UMC", true, LOG_LEVEL_DEBUG);
	log_info(logger, "Log creado");
}
void cargarArchivoConfig(){
	inicializarLog();
	t_config * config_cpu = config_create("../resources/config.cfg");

	if( config_cpu == NULL )
	{
		log_error(logger, "No se pudo cargar el archivo de configuración");
		abort();
	}

	// OBTENGO CONFIGURACION DEL CONFIG /
	miConfig.ipSWAP = config_get_string_value(config_cpu, "IP_SWAP" );
	char* algor = config_get_string_value(config_cpu, "ALGORITMO" );
	if(string_equals_ignore_case(algor,"CLOCK")){
		miConfig.algoritmoPlanificacion = 0;
		log_info(logger, "El algoritmo es Clock");
	}else if (string_equals_ignore_case(algor,"CLOCKMODIFICADO")){
		miConfig.algoritmoPlanificacion = 1;
		log_info(logger, "El algoritmo es Clock Modificado");
	}else{
		log_error(logger, "No se reconoce el algoritmo");
	}

	miConfig.puertoEscucha= config_get_string_value(config_cpu, "PUERTO");
	miConfig.entradasTLB = config_get_int_value(config_cpu, "ENTRADAS_TLB");
	miConfig.maximoMarcoXProceso = config_get_int_value(config_cpu, "MAXIMO_MARCOS_POR_PROCESO");
	miConfig.cantidadMarcos = config_get_int_value(config_cpu, "CANTIDAD_MARCOS");
	miConfig.puertoSWAP = config_get_string_value(config_cpu, "PUERTO_SWAP");
	miConfig.retardoMemoria = config_get_int_value(config_cpu, "RETARDO_MEMORIA");
	miConfig.tamanioMarco = config_get_int_value(config_cpu, "TAMANIO_MARCO");

	if (miConfig.entradasTLB == 0) {
		miConfig.usoTLB = false;
		log_info(logger,"NO HAY TLB");
	}else{
		log_info(logger,"USO TLB TAMAÑO: %d",miConfig.entradasTLB);
		miConfig.usoTLB = true;
	}

	free(config_cpu);
	log_trace(logger, "Archivo de configuración cargado");


}

void inicializar(){
	//Reservo la memoria
	memoria = mallocCommon(miConfig.cantidadMarcos * miConfig.tamanioMarco);
	log_info(logger,"LA MEMORIA SOLICITADA EMPIEZA EN: %p Y TERMINA EN: %p", memoria,memoria + (miConfig.cantidadMarcos * miConfig.tamanioMarco));
	//cargo los marcos en la lista
	listaMarcos=list_create();
	int i;
	for ( i = 0; i < miConfig.cantidadMarcos; ++i) {
		t_marco* marco = mallocCommon(sizeof(t_marco));
		marco->comienzo=mallocCommon(miConfig.tamanioMarco);
		marco->id = i;
		marco->comienzo = memoria + i*miConfig.tamanioMarco;
		marco->asignado = false;
		list_add(listaMarcos,(void*)marco);
	}
	log_info(logger,"Lista de Marcos creada");
	//Inicializo listas
	listaTablasdePaginas = list_create(); //tablas de paginas de los procesos
	listaPaginasMemoria = list_create(); //Datos de pags en memoria para algoritmo de reemplazo
	//inicio la TLB


	listaTLB = list_create();

	//Inicializo los semaforos que voy a usar al recibir mensajes
	pthread_mutex_init(&semaforoGlobal, NULL );
	pthread_mutex_init(&semaforoSWAP, NULL );
}

/*
 * SOLO VERIFICO si la pagina esta cargada en memoria
 * comparo el id con la tabla del proceso ya cargada
 * devuelvo direccion o -1
 */
char* estaPaginaEnMemoria(t_tablaPaginas* paginasProcesoActivo, int idPagina){
	aplicarRetardo("comprobando pagina en memoria");
	t_entradaPagina* datosPagina;
	//obtengo la entrada de la pagina
	datosPagina= obtenerEntradaDePagina(paginasProcesoActivo, idPagina);
	bool tablaNumeroPagina(t_entradaPagina* tabla){
		return tabla->numeroPagina==idPagina;
	}
	datosPagina = (t_entradaPagina*)list_find(paginasProcesoActivo->paginas, (void*)tablaNumeroPagina);
	if(datosPagina!=NULL && datosPagina->presencia == 1){
		//busco el marco en la lista de marcos
		bool marcoPorNumero(t_marco* marco){
			log_info(logger,"marco por numero", marco->id);
			return marco->id==datosPagina->frame;
		}
		t_marco* marco = mallocCommon(sizeof(marco));
		marco = (t_marco*)list_find(listaMarcos,(void*)marcoPorNumero);
		//free(datosPagina);
		return marco->comienzo;
	}else{
		return "-1";
	}
}

/*
 * SOLO VERIFICO
 * devuelve la direccion si esta en la TLB, sino devuelve -1
 */
char* estaPaginaEnTLB(int idProc, int idPagina){

	int i;
	char* direccion = string_new();
	string_append(&direccion,"-1");
	if(miConfig.usoTLB){
		t_entradaTLB* entrada = mallocCommon(sizeof(t_entradaTLB));
		for (i= 0; i < list_size(listaTLB); ++i) {

			entrada = (t_entradaTLB*)list_get(listaTLB,i);
			//log_info(logger,"La Pagina: %d PID: %d Se encuentra en la TLB",idPagina,idProc);
			if(entrada->pid==idProc &&  entrada->pagina == idPagina){

				direccion = entrada->direccion;
				log_info(logger,"La Pagina: %d PID: %d Se encuentra en la TLB",idPagina,idProc);
			}
			//free(entrada);
		}
	}return direccion;
}

void finalizoProgramaEnTLB(int idProg){
	bool cerrarProgramas(t_entradaTLB* eTLB){
		return eTLB->pid==idProg;
	}
	if(miConfig.usoTLB){
		while(list_any_satisfy(listaTLB,(void*)cerrarProgramas)){
			list_remove_by_condition(listaTLB,(void*)cerrarProgramas);
			log_info(logger,"Se elimino entrada de tlb. programa finalizado");
		}
	}
}


/*	agregarPaginaEnMemoria
 * 	recibe la tabla de paginas del proceso, el numero de pagina y la data
 * 	Le asigna un marco donde cargarla, tomando la direccion de dicho marco
 * 	Si tiene menos de maximo por proceso, veo si hay disponible.
 * 	Si tiene el maximo reemplazo por clock o clock modificado, usando la lista con los datos de pags del proceso.
 * 	De ser necesario guardo la pagina que reemplazaré
 * 	Actualiza la tabla de paginas del proceso
 * 	Agrega la pagina a la TLB
*/
bool marcosNoAsignados(t_marco* marco){
	return marco->asignado==false;
}

char* agregarPaginaEnMemoria(t_tablaPaginas* tablaProceso, int idPagina, void* data){
	aplicarRetardo("Agregar pagina en memoria");
	//Obtengo entrada de pagina deseada
	t_entradaPagina* entradaTP = mallocCommon(sizeof(t_entradaPagina));
	entradaTP = (t_entradaPagina*)list_get(tablaProceso->paginas, idPagina);
	//Obtengo los datos de paginas en memoria del proceso, para reemplazo
	t_datosCargaMemoria* datosProcActual = mallocCommon(sizeof(t_datosCargaMemoria));
	bool mismoPIDPaginasMemoria(t_datosCargaMemoria* dcmtmp){
		return dcmtmp->pid==tablaProceso->PID;
	}

	datosProcActual = (t_datosCargaMemoria*)list_find(listaPaginasMemoria, (void*)mismoPIDPaginasMemoria);

	t_datosxPaginas* datosxPagina=mallocCommon(sizeof(t_datosxPaginas));
	//Asigno marco y lo cargo ahi
	t_marco* marcoAAsignar;// = mallocCommon(sizeof(t_marco));
	//marcoAAsignar->comienzo=mallocCommon(miConfig.tamanioMarco);
	int idPaginaAReemplazar=-1;
	bool asigno = false;
	if(tablaProceso->marcosAsignados<miConfig.maximoMarcoXProceso){

		if(list_any_satisfy(listaMarcos,(void*) marcosNoAsignados)){
			//Si tiene menos marcos del maximo busco el disponible
			marcoAAsignar = (t_marco*)list_find(listaMarcos,(void*)marcosNoAsignados);
			log_info(logger,"Marco a asignar id %d", marcoAAsignar->id);
			datosxPagina->clock=true;
			datosxPagina->clockModificado=false;
			datosxPagina->idPag=idPagina;
			datosProcActual->puntero++;
			list_add_in_index(datosProcActual->datosxPaginas,datosProcActual->puntero,(void*)datosxPagina);
			tablaProceso->marcosAsignados++;
			asigno=true;

		}else if(tablaProceso->marcosAsignados == 0){
			//Problema al querer cargar pagina del programa y no hay marcos disponibles, devuelvo -1
			return "-1";
		}
	}
	if(!asigno){
		//Ya tiene el maximo de marcos o no hay ninguno sin asignar, reemplazo una de las paginas por clock (0) o clock modificado(1)
		if(miConfig.algoritmoPlanificacion==0){  //******CLOCK*********
			int i;
			bool conClockTrue(t_datosxPaginas* dxp){
				return dxp->clock==true;
			}
			if (list_all_satisfy(datosProcActual->datosxPaginas,(void*)conClockTrue)) {
				for (i = 0; i < list_size(datosProcActual->datosxPaginas); ++i) {
					datosxPagina= (t_datosxPaginas*)list_get(datosProcActual->datosxPaginas,i);
					datosxPagina->clock=false;
					list_replace(datosProcActual->datosxPaginas,i,(void*)datosxPagina);
				}
			}
			//if(datosProcActual->puntero>=list_s)
			while(idPaginaAReemplazar==-1) {
				//Busco la pagina a reemplazar
				datosxPagina= (t_datosxPaginas*)list_get(datosProcActual->datosxPaginas,datosProcActual->puntero);
				if(datosxPagina->clock==false){
					idPaginaAReemplazar = datosxPagina->idPag;
					datosxPagina->clock=true;
					datosxPagina->clockModificado=false;
					datosxPagina->idPag = idPagina;
					list_replace(datosProcActual->datosxPaginas,datosProcActual->puntero,(void*)datosxPagina);
				}
				if(datosProcActual->puntero==list_size(datosProcActual->datosxPaginas)-1){
					datosProcActual->puntero=0;
				}else{
					datosProcActual->puntero++;
				}
			}
		}else if(miConfig.algoritmoPlanificacion==1){ //******CLOCK MODIFICADO*********
			bool conClockYModificadoFalse(t_datosxPaginas* dxp){
				return (dxp->clock==false && dxp->clockModificado==false);
			}
			bool conClockFalseYModificadoTrue(t_datosxPaginas* dxp){
				return (dxp->clock==false && dxp->clockModificado==true);
			}
			while(idPaginaAReemplazar==-1){
				//Veo si existe alguna U=0 y M=0
				if(list_any_satisfy(datosProcActual->datosxPaginas,(void*)conClockYModificadoFalse)){
					while(idPaginaAReemplazar==-1){
						//Busco la pagina a reemplazar
						datosxPagina= (t_datosxPaginas*)list_get(datosProcActual->datosxPaginas,datosProcActual->puntero);
						if(datosxPagina->clock==false && datosxPagina->clockModificado==false){
							idPaginaAReemplazar = datosxPagina->idPag;
							datosxPagina->clock=true;
							datosxPagina->clockModificado=false;
							datosxPagina->idPag = idPagina;
							list_replace(datosProcActual->datosxPaginas,datosProcActual->puntero,(void*)datosxPagina);
						}
						if(datosProcActual->puntero==list_size(datosProcActual->datosxPaginas)-1){
							datosProcActual->puntero=0;
						}else{
							datosProcActual->puntero++;
						}
					}
				//Veo si existe alguna U=0 y M=1
				}else if(list_any_satisfy(datosProcActual->datosxPaginas,(void*)conClockFalseYModificadoTrue)){
					while(idPaginaAReemplazar==-1){

						datosxPagina= (t_datosxPaginas*)list_get(datosProcActual->datosxPaginas,datosProcActual->puntero);
						if(datosxPagina->clock==false && datosxPagina->clockModificado==true){
							idPaginaAReemplazar = datosxPagina->idPag;
							datosxPagina->clock=true;
							datosxPagina->clockModificado=false;
							datosxPagina->idPag = idPagina;
							list_replace(datosProcActual->datosxPaginas,datosProcActual->puntero,(void*)datosxPagina);
						}else{
							//Cambio U=0 y avanzo
							datosxPagina->clock=false;
							list_replace(datosProcActual->datosxPaginas,datosProcActual->puntero,(void*)datosxPagina);
						}
						if(datosProcActual->puntero==list_size(datosProcActual->datosxPaginas)-1){
							datosProcActual->puntero=0;
						}else{
							datosProcActual->puntero++;
						}
					}
				}else{
					int i;
					for (i = 0; i < list_size(datosProcActual->datosxPaginas); ++i) {
						datosxPagina= (t_datosxPaginas*)list_get(datosProcActual->datosxPaginas,i);
						datosxPagina->clock=false;
						list_replace(datosProcActual->datosxPaginas,i,(void*)datosxPagina);
					}
				}
			}



		}else log_error(logger, "NO HAY UN ALGORITMO DE PLANIFICACION CORRECTO");
	}
	//Si debo reemplazar una pagina, actualizo la entrada de dicha pagina
	if(idPaginaAReemplazar!=-1){
		log_info(logger,"Es necesario reemplazo de pagina id: %d",idPaginaAReemplazar);
		t_entradaPagina* entradaPaginaAReemplazar = obtenerEntradaDePagina(tablaProceso,idPaginaAReemplazar);
		//Si fue modificada debo guardarla primero, envio datos a swap

		marcoAAsignar = (t_marco*)list_get(listaMarcos,entradaPaginaAReemplazar->frame);

		if(entradaPaginaAReemplazar->modificacion==true){
			//ENVIAR DATOS A SWAP
			t_envio* mensaje = pedirPaquete(&tablaProceso->PID, ESCRIBIR_PAGINA_SWAP,sizeof(int));
			aniadirAlPaquete(mensaje, &idPaginaAReemplazar,sizeof(int));
			void* dataReemplazar = mallocCommon(miConfig.tamanioMarco);
			memcpy(dataReemplazar,marcoAAsignar->comienzo,miConfig.tamanioMarco);
			aniadirAlPaquete(mensaje,marcoAAsignar->comienzo,miConfig.tamanioMarco);

			pthread_mutex_lock(&semaforoSWAP);
			enviar(socketSWAP,mensaje,NULL);
			destruirPaquete(mensaje);
			mensaje=recibir(socketSWAP,NULL);
			pthread_mutex_unlock(&semaforoSWAP);
			if(mensaje->codigo_Operacion==ESCRIBIR_PAGINA_SWAP_OK){
				log_info(logger,"Pagina: %d almacenada en swap correctamente antes de reemplazo",idPaginaAReemplazar);
				//puts(dataReemplazar);
			}
		}

		entradaPaginaAReemplazar->presencia=false;
		list_replace(tablaProceso->paginas,idPaginaAReemplazar,(void*)entradaPaginaAReemplazar);
		if(!string_equals_ignore_case(estaPaginaEnTLB(tablaProceso->PID,idPaginaAReemplazar),"-1")){
			log_info(logger,"Saco pagina %d de TLB", idPaginaAReemplazar);
			bool mismoPIDTLB(t_entradaTLB* etlb){
				//log_info(logger,"pid tlb %s  pid pag %s",etlb->programaPagina,pidpag);
				return etlb->pid ==tablaProceso->PID && etlb->pagina==idPaginaAReemplazar;
			}
			list_remove_by_condition(listaTLB,(void*)mismoPIDTLB);
		}
		//free(entradaPaginaAReemplazar);
	}
	marcoAAsignar->comienzo=malloc(miConfig.tamanioMarco);
	memcpy(marcoAAsignar->comienzo,data, miConfig.tamanioMarco); //Guardo la información en la memoria
	marcoAAsignar->asignado = true;
	list_replace(listaMarcos,marcoAAsignar->id,(void*)marcoAAsignar);




	//actualizo la tabla de paginas
	entradaTP->frame = marcoAAsignar->id;
	entradaTP->presencia = 1;
	entradaTP->modificacion=false;
	list_replace(tablaProceso->paginas, idPagina,(void*)entradaTP);



	//free(entradaTP);
	char* direccionPagina = string_new();
	direccionPagina=marcoAAsignar->comienzo;
	//free(marcoAAsignar);
	return direccionPagina;

}

/* Algoritmo LRU
 * Verifico si ya esta la entrada en la lista, sino
 * Si la TLB está llena, elimino la ultima
 * Siempre la agrego primera
 */
void actualizarEntradaEnTLB(t_entradaTLB* entradaNueva){
	bool comparoEntradas(t_entradaTLB* unaEntrada){
		return unaEntrada->pid== entradaNueva->pid && entradaNueva->pagina == unaEntrada->pagina;
	}
	if(miConfig.usoTLB){
		if(list_any_satisfy(listaTLB,(void*)comparoEntradas)){
			//La entrada ya esta en la TLB, la pongo como primera (hay una forma mejor?)
			t_entradaTLB* entradatmp = mallocCommon(sizeof(t_entradaTLB));
			entradatmp = (t_entradaTLB*)list_find(listaTLB, (void*)comparoEntradas);
			list_remove_by_condition(listaTLB, (void*)comparoEntradas);
			list_add_in_index(listaTLB,0,(void*)entradatmp);
			//free(entradatmp);
		}else{
			if(list_size(listaTLB)==miConfig.entradasTLB){
				log_info(logger,"Tamaño tlb maximo %d elimino en posicion %d",miConfig.entradasTLB,list_size(listaTLB)-1);
				list_remove(listaTLB,list_size(listaTLB)-1);
			}
			list_add_in_index(listaTLB, 0, (void*)entradaNueva);
		}
		log_info(logger,"TLB actualizada. Prog: %d Pag: %d",entradaNueva->pid,entradaNueva->pagina);
	}
}

/*
 * leerPagina - Comprobar primero que la pagina este en memoria!
 * Devuelve lo almacenado en memoria, partiendo del comienzo del marco asignado a la pagina
 * Actualiza clock para reemplazo
 */
void* leerPagina(t_tablaPaginas* tablaProceso,t_datosCargaMemoria* datosCM, int idPagina, int offset, int tamanio){
	t_entradaPagina* entradaPagina = obtenerEntradaDePagina(tablaProceso, idPagina);
	t_marco* marcoPagina = mallocCommon(sizeof(t_marco));
	marcoPagina = (t_marco*)list_get(listaMarcos,entradaPagina->frame);
	void* data=mallocCommon(tamanio);
	char* base;// = mallocCommon(string_length(marcoPagina->comienzo)+offset);
	base = marcoPagina->comienzo + offset;
	memcpy(data, base, tamanio);

	t_datosxPaginas* datosXP=mallocCommon(sizeof(t_datosxPaginas));
	bool encontro =true;
	int i = 0;
	while(encontro == false){
		datosXP = (t_datosxPaginas*)list_get(datosCM->datosxPaginas,i);
		if(datosXP->idPag==idPagina){
			datosXP->clock=true;
			list_replace(datosCM->datosxPaginas, i, (void*)datosXP);
			encontro = true;
		}
		i++;
	}
	//free(datosXP);
	//free(entradaPagina);
	return data;
}

/*
 * escribirPagina - Comprobar primero que la pagina este en memoria!
 * copia en memoria la data y actualiza modificado para clock y en tabla de paginas del proceso
 */

void escribirPagina(t_tablaPaginas* tablaProceso,t_datosCargaMemoria* datosCM, int idPagina, int offset, int tamanio, void*data, char* dirPagPLE){
	t_entradaPagina* entradaPag = mallocCommon(sizeof(t_entradaPagina));
	int j = -1;
	bool encuentro = false;
	while(encuentro == false){
		j++;
		entradaPag = (t_entradaPagina*)list_get(tablaProceso->paginas,j);
		if(entradaPag->numeroPagina==idPagina)
			encuentro=true;
	}

	memcpy(dirPagPLE+offset, data, tamanio);
	entradaPag->modificacion=true;
	list_replace(tablaProceso->paginas,j,(void*)entradaPag);
	t_datosxPaginas* datosXP=mallocCommon(sizeof(t_datosxPaginas));

	bool encontro =true;
	int i = 0;
	while(encontro == false){
		datosXP = (t_datosxPaginas*)list_get(datosCM->datosxPaginas,i);
		if(datosXP->idPag==idPagina){
			datosXP->clock=true;
			datosXP->clockModificado = true;
			list_replace(datosCM->datosxPaginas, i, (void*)datosXP);
			encontro = true;
		}
		i++;
	}
	//free(datosXP);
	//free(entradaPag);


}

t_entradaPagina* obtenerEntradaDePagina(t_tablaPaginas* tp, int id){
	//aplicarRetardo("obtener Entrada de pagina");
	bool entradaDeLaPagina(t_entradaPagina* entrada){

		return entrada->numeroPagina==id;
	}
	t_entradaPagina* ep = mallocCommon(sizeof(t_entradaPagina));
	return ep = (t_entradaPagina*)list_find(tp->paginas,(void*)entradaDeLaPagina);
}


char* obtenerDireccionDePagina(t_tablaPaginas* tp, int idP, char* dirPagPLE){

	dirPagPLE = estaPaginaEnMemoria(tp,idP);
	if(string_equals_ignore_case(dirPagPLE,"-1")){
		//Fallo de pagina, no se encuentra en memoria
		log_warning(logger,"Fallo de pagina - Proceso: %d Pagina: %d", tp->PID,idP);
		void* contenidoPagina = mallocCommon(miConfig.tamanioMarco);
		//Pedir a swap el contenido de la pagina
		t_envio* mensaje;
		mensaje = pedirPaquete(&tp->PID,LEER_PAGINA_SWAP,sizeof(int));
		aniadirAlPaquete(mensaje,&idP,sizeof(int));
		pthread_mutex_lock(&semaforoSWAP);
		enviar(socketSWAP,mensaje,NULL);
		destruirPaquete(mensaje);
		mensaje = recibir(socketSWAP,NULL);
		memcpy(contenidoPagina,mensaje->data,miConfig.tamanioMarco);
		pthread_mutex_unlock(&semaforoSWAP);
		dirPagPLE = agregarPaginaEnMemoria(tp,idP,contenidoPagina);
		if(string_equals_ignore_case(dirPagPLE,"-1")){
			log_error(logger, "No hay marcos disponibles para el proceso: %d", tp->PID);
		}else{
			log_info(logger, "Pagina cargada en memoria correctamente - Proceso: %d Pagina: %d Data:", tp->PID,idP);
			//puts(contenidoPagina);
			//actualizo la tlb
			t_entradaTLB* entradaTLB = mallocCommon(sizeof(t_entradaTLB));
			entradaTLB->pid = tp->PID;
			entradaTLB->pagina=idP;
			entradaTLB->direccion = dirPagPLE;

			actualizarEntradaEnTLB(entradaTLB);

			//free(entradaTLB);

		}
	}

	return dirPagPLE;
}


void hiloCPU(void * socket){
	int anterior=-1;
	int PIDProcesoActivo=-1;
	int socketCPU;
	char* direccionPaginaParaLE;
	memcpy(&socketCPU, socket, sizeof(int));
	//free(socket);
	t_tablaPaginas* tablaProcesoActivo = mallocCommon(sizeof(t_tablaPaginas));
	t_datosCargaMemoria* paginasEnMemoriaProcActivo = mallocCommon(sizeof(t_datosCargaMemoria));
	while(1){
		t_envio *mensaje;
		bool buscarTablaPrograma(t_tablaPaginas* unaTablaDePags){
			return unaTablaDePags->PID==PIDProcesoActivo;
		}
		bool buscarDatosPaginasMemoria(t_datosCargaMemoria* dcm){
			return dcm->pid == PIDProcesoActivo;
		}
		void pierdeConexionCPU(){


			log_info(logger, "Se perdio la conexion con CPU: %d", socketCPU);

			pthread_mutex_lock(&semaforoGlobal);
			if(PIDProcesoActivo>=0){
				//Guardo las estructuras del proceso actual
				list_remove_by_condition(listaTablasdePaginas,(void*)buscarTablaPrograma);
				list_add(listaTablasdePaginas, (void*)tablaProcesoActivo);
				list_remove_by_condition(listaPaginasMemoria,(void*)buscarDatosPaginasMemoria);
				list_add(listaPaginasMemoria,(void*)paginasEnMemoriaProcActivo);
			}

			//free(tablaProcesoActivo);
			//free(paginasEnMemoriaProcActivo);
			close(socketCPU);
			FD_CLR(socketCPU, &descriptoresLectura);
			pthread_mutex_unlock(&semaforoGlobal);
			pthread_exit(NULL );
		}
		mensaje = recibir(socketCPU, (void*)pierdeConexionCPU );
		direccionPaginaParaLE=string_new();
		string_append(&direccionPaginaParaLE,"-1");
		int idPaginaEscribir;
		u_int32_t offsetEscribir, tamanioEscribir;
		void* dataEscribir;
		void* dataLeer;
		int idPaginaLeer, offsetLeer, tamanioLeer;

		switch (mensaje->codigo_Operacion) {

			case CAMBIO_PROCESO_ACTIVO_CPU:
				pthread_mutex_lock(&semaforoGlobal);
				int pidACambiar;
				memcpy(&pidACambiar, mensaje->data, sizeof(int));
				log_info(logger,"Se pide cambio a proceso: %d",pidACambiar);
				if(pidACambiar==PIDProcesoActivo){
					log_info(logger,"Se pide cambio a proceso ya activo: %d",PIDProcesoActivo);
				}else{
					if(PIDProcesoActivo>=0){
						//Guardo las estructuras del proceso actual
/*
						list_remove_by_condition(listaTablasdePaginas,(void*)buscarTablaPrograma);
						list_add(listaTablasdePaginas, (void*)tablaProcesoActivo);
						list_remove_by_condition(listaPaginasMemoria,(void*)buscarDatosPaginasMemoria);
						list_add(listaPaginasMemoria,(void*)paginasEnMemoriaProcActivo);
	*/					anterior = PIDProcesoActivo;
					}

					//Actualizo las estructuras para el proceso nuevo
					PIDProcesoActivo=pidACambiar;
					tablaProcesoActivo = (t_tablaPaginas*)list_find(listaTablasdePaginas, (void*)buscarTablaPrograma);
					paginasEnMemoriaProcActivo = (t_datosCargaMemoria*)list_find(listaPaginasMemoria, (void*)buscarDatosPaginasMemoria);

					log_info(logger,"Se cambio el proceso activo. Anterior: %d - Nuevo: %d", anterior, PIDProcesoActivo);
				}
				pthread_mutex_unlock(&semaforoGlobal);
				destruirPaquete(mensaje);
				mensaje = pedirPaquete(strdup("OK"), CAMBIO_PROCESO_ACTIVO_CPU_OK, 3);
				enviar(socketCPU, mensaje, NULL );
				destruirPaquete(mensaje);
				break;


			case ESCRIBIR_PAGINA_CPU:

				memcpy(&idPaginaEscribir, mensaje->data, sizeof(int));
				memcpy(&offsetEscribir, mensaje->data + sizeof(int), sizeof(u_int32_t));
				memcpy(&tamanioEscribir, mensaje->data + sizeof(int) + sizeof(u_int32_t),sizeof(u_int32_t));


				//Verifico cuantas paginas ocupa la escritura
				int paginasNecesarias = 1;
				int tam=0;
				int yaEscrito=0;
				int offTam =offsetEscribir+tamanioEscribir;
				//log_info(logger,"off %d tam %d offTam %d  tamanioMarco %d",offsetEscribir,tamanioEscribir,offTam,miConfig.tamanioMarco);
				while(offTam>miConfig.tamanioMarco){
					offTam-=miConfig.tamanioMarco;
					paginasNecesarias++;
					//log_info(logger,"offTam %d  paginasNecesarias %d",offTam,paginasNecesarias);
				}
				log_info(logger,"paginas necesarias %d",paginasNecesarias);
				dataEscribir = mallocCommon(miConfig.tamanioMarco*paginasNecesarias);
				memset(dataEscribir,'\0',miConfig.tamanioMarco*paginasNecesarias);
				memcpy(dataEscribir,mensaje->data + sizeof(int) + sizeof(int) + sizeof(int),tamanioEscribir);
				int i;
				for (i = 0; i < paginasNecesarias; ++i) {
					idPaginaEscribir+=i;
					if(i>0)offsetEscribir=0;
					if(i==(paginasNecesarias-1)){
						tam=tamanioEscribir-yaEscrito-1;
					}else{ tam=miConfig.tamanioMarco-offsetEscribir;}

					pthread_mutex_lock(&semaforoGlobal);
					//Obtengo la direccion
					direccionPaginaParaLE = estaPaginaEnTLB(tablaProcesoActivo->PID,idPaginaEscribir);
					if(string_equals_ignore_case(direccionPaginaParaLE,"-1")){
						log_info(logger,"Comparo largo tabla paginas %d con id pag %d",list_size(tablaProcesoActivo->paginas),idPaginaEscribir);
						if(list_size(tablaProcesoActivo->paginas) > idPaginaEscribir){
							direccionPaginaParaLE = obtenerDireccionDePagina(tablaProcesoActivo, idPaginaEscribir,direccionPaginaParaLE);
						}else{
							log_error(logger, "Problema al escribir en Proceso: %d  STACKOVERFLOW", tablaProcesoActivo->PID);
							//Envio error a CPU
							mensaje = pedirPaquete(strdup("NL"),NUEVO_PROGRAMA_NO_HAY_MARCOS,3);
							i=paginasNecesarias;
						}
					}
					pthread_mutex_unlock(&semaforoGlobal);
					destruirPaquete(mensaje);
					if(string_equals_ignore_case(direccionPaginaParaLE,"-1")){
						log_error(logger, "Problema al escribir en Proceso: %d", tablaProcesoActivo->PID);
						//Envio error a CPU
						mensaje = pedirPaquete(strdup("NL"),NUEVO_PROGRAMA_NO_HAY_MARCOS,3);
						i=paginasNecesarias;
					}else{
						pthread_mutex_lock(&semaforoGlobal);
						escribirPagina(tablaProcesoActivo,paginasEnMemoriaProcActivo,idPaginaEscribir,offsetEscribir,tam,dataEscribir,direccionPaginaParaLE);
						pthread_mutex_unlock(&semaforoGlobal);
						yaEscrito+=tam;
						//log_info(logger,"Ya escrito %d  tamaño %d",yaEscrito,tamanioEscribir);
						//Envio confirmacion a CPU
						if(i==0)mensaje = pedirPaquete(strdup("OK"),ESCRIBIR_PAGINA_CPU_OK,3);
						log_info(logger, "Escritura correcta. Proceso: %d Pagina: %d Offset: %d", tablaProcesoActivo->PID, idPaginaEscribir,offsetEscribir);
						//puts(dataEscribir);
					}
				}
				free(dataEscribir);
				enviar(socketCPU,mensaje,NULL);
				destruirPaquete(mensaje);
				break;


			case LEER_PAGINA_CPU:
				memcpy(&idPaginaLeer, mensaje->data, sizeof(int));
				memcpy(&offsetLeer, mensaje->data + sizeof(int), sizeof(int));
				memcpy(&tamanioLeer, mensaje->data + sizeof(int) + sizeof(int),sizeof(int));
				destruirPaquete(mensaje);
				dataLeer=mallocCommon(tamanioLeer);
				int pN = 1;
				int tamLeer=0;
				int yaLeido=0;
				int offTamLeer =offsetLeer+tamanioLeer;
				while(offTamLeer>miConfig.tamanioMarco){
					offTamLeer-=miConfig.tamanioMarco;
					pN++;
				}

				int j;
				for (j = 0; j < pN; ++j) {
					idPaginaLeer+=j;
					if(j>0){
						offsetLeer=0;
					}
					if(j==(pN-1)){
						tamLeer=tamanioLeer-yaLeido;
					}else{ tamLeer=miConfig.tamanioMarco-offsetLeer;
					}
					pthread_mutex_lock(&semaforoGlobal);
					//Obtengo la direccion
					direccionPaginaParaLE = estaPaginaEnTLB(tablaProcesoActivo->PID, idPaginaLeer);

					if(string_equals_ignore_case(direccionPaginaParaLE,"-1")){
						if(list_size(tablaProcesoActivo->paginas) > idPaginaEscribir){

							direccionPaginaParaLE = obtenerDireccionDePagina(tablaProcesoActivo, idPaginaLeer,direccionPaginaParaLE);
						}else{
							log_error(logger, "Problema al escribir en Proceso: %d  STACKOVERFLOW", tablaProcesoActivo->PID);
							//Envio error a CPU
							mensaje = pedirPaquete(strdup("NL"),NUEVO_PROGRAMA_NO_HAY_MARCOS,3);
							j=paginasNecesarias;
						}
					}


					pthread_mutex_unlock(&semaforoGlobal);
					if(string_equals_ignore_case(direccionPaginaParaLE,"-1")){
						log_error(logger, "Problema al leer en Proceso: %d", tablaProcesoActivo->PID);
						//Envio error a CPU
						mensaje = pedirPaquete(strdup("NL"),NUEVO_PROGRAMA_NO_HAY_MARCOS,3);
						j=pN;
					}else{

						pthread_mutex_lock(&semaforoGlobal);
						void* dataLeerTMP=leerPagina(tablaProcesoActivo,paginasEnMemoriaProcActivo,idPaginaLeer,offsetLeer,tamLeer);
						pthread_mutex_unlock(&semaforoGlobal);
						memcpy(dataLeer+yaLeido,dataLeerTMP,tamLeer);
						free(dataLeerTMP);
						//Envio la información a CPU
						if(j==pN-1)
							mensaje = pedirPaquete(dataLeer,LEER_PAGINA_CPU_OK,tamanioLeer);
						log_info(logger, "Lectura correcta. Proceso: %d Pagina: %d Offset: %d tamanio: %d", tablaProcesoActivo->PID,idPaginaLeer, offsetLeer, tamanioLeer);
						yaLeido+=tamLeer;
						//log_info(logger,"Ya leido %d  tamaño %d",yaLeido,tamanioLeer);
					}
				}
				enviar(socketCPU,mensaje,NULL);

				//puts(dataLeer);
				free(dataLeer);
				break;


			case CAE_CPU:
				log_info(logger, "Se perdio la conexion con CPU: %d", socketCPU);
				destruirPaquete(mensaje);
				pthread_mutex_lock(&semaforoGlobal);
				if(PIDProcesoActivo!=-1){
					//Guardo las estructuras del proceso actual
					list_remove_by_condition(listaTablasdePaginas,(void*)buscarTablaPrograma);
					list_add(listaTablasdePaginas, (void*)tablaProcesoActivo);
					list_remove_by_condition(listaPaginasMemoria,(void*)buscarDatosPaginasMemoria);
					list_add(listaPaginasMemoria,(void*)paginasEnMemoriaProcActivo);
				}

				//free(tablaProcesoActivo);
				//free(paginasEnMemoriaProcActivo);
				//close(socketCPU);
				FD_CLR(socketCPU, &descriptoresLectura);
				pthread_mutex_unlock(&semaforoGlobal);
				pthread_exit(NULL );
				break;


			default:

				log_warning(logger, "Codigo invalido desde CPU: %d", socketCPU);
				mensaje = pedirPaquete(strdup("FF"), ERROR_CODIGO_INVALIDO, 3);
				enviar(socketCPU, mensaje, NULL );
				break;

		}



	}

}

void hiloKernel(void* socket){
	int i;
	int socketNucleo;
	memcpy(&socketNucleo,socket,sizeof(int));
	//free(socket);
	while(1){
		t_envio* mensaje;
		void errorRecibir(){
			log_error(logger,"Error al recibir mensaje de Nucleo. Cerrando conexion...");
			close(socketNucleo);
			FD_CLR(socketNucleo, &descriptoresLectura);
			pthread_exit(NULL );
		}

		mensaje = recibir(socketNucleo,(void*)errorRecibir);

		int idProgramaNuevo, numeroPaginas;
		void* data;
		int idProgramaEliminar;
		t_tablaPaginas* tablaFinPrograma = mallocCommon(sizeof(t_tablaPaginas));
		bool buscarTablaPrograma(t_tablaPaginas* unaTablaDePags){
			return unaTablaDePags->PID==idProgramaEliminar;
		}
		switch(mensaje->codigo_Operacion){

		case NUEVO_PROGRAMA:

			memcpy(&idProgramaNuevo,mensaje->data,sizeof(int));
			memcpy(&numeroPaginas,mensaje->data+sizeof(int),sizeof(int));
			int tamanioTotalData = mensaje->data_size-(2*sizeof(int));
			data = mallocCommon(tamanioTotalData);
			memcpy(data,mensaje->data+(2*sizeof(int)),tamanioTotalData);
			destruirPaquete(mensaje);
			//pregunto a swap si hay lugar para x número de paginas
			mensaje = pedirPaquete(&idProgramaNuevo,NUEVO_PROGRAMA_SWAP,sizeof(int));
			aniadirAlPaquete(mensaje,&numeroPaginas,sizeof(int));
			pthread_mutex_lock(&semaforoSWAP);
			enviar(socketSWAP,mensaje,NULL);
			destruirPaquete(mensaje);
			mensaje = recibir(socketSWAP,NULL);
			pthread_mutex_unlock(&semaforoSWAP);
			if(mensaje->codigo_Operacion==NUEVO_PROGRAMA_SWAP_OK){
				t_tablaPaginas* tablaProceso = mallocCommon(sizeof(t_tablaPaginas));
				tablaProceso->PID=idProgramaNuevo;
				tablaProceso->marcosAsignados=0;
				//Cargo en la lista para algoritmo de cambio de paginas
				t_datosCargaMemoria* datosNuevoProc = mallocCommon(sizeof(t_datosCargaMemoria));
				datosNuevoProc->datosxPaginas=list_create();
				datosNuevoProc->puntero=-1;
				datosNuevoProc->pid=idProgramaNuevo;
				pthread_mutex_lock(&semaforoGlobal);
				list_add(listaPaginasMemoria, (void*) datosNuevoProc);
				pthread_mutex_unlock(&semaforoGlobal);
				t_list* listaPaginas = list_create();
				int tamYaEscrito=0;
				int restante = tamanioTotalData;
				for (i = 0; i < numeroPaginas; ++i) {
					restante=tamanioTotalData-tamYaEscrito;
					destruirPaquete(mensaje);
					void* dataEscribir = mallocCommon(miConfig.tamanioMarco);
					memset(dataEscribir,'\0',miConfig.tamanioMarco);
					if(restante>0){
						if(restante>miConfig.tamanioMarco){
							memcpy(dataEscribir,data+tamYaEscrito,miConfig.tamanioMarco);
							tamYaEscrito+=miConfig.tamanioMarco;
						}else{
							memcpy(dataEscribir,data+tamYaEscrito,restante);
							tamYaEscrito+=restante;
						}
					}

					//envío a swap que almacene las paginas
					//puts(dataEscribir);
					mensaje = pedirPaquete(&idProgramaNuevo,ESCRIBIR_PAGINA_SWAP,sizeof(int));
					aniadirAlPaquete(mensaje,&i,sizeof(int));
					aniadirAlPaquete(mensaje,dataEscribir,miConfig.tamanioMarco);
					pthread_mutex_lock(&semaforoSWAP);
					log_info(logger,"EnvioPag a swap");
					enviar(socketSWAP,mensaje,NULL);
					destruirPaquete(mensaje);
					mensaje = recibir(socketSWAP,NULL);
					pthread_mutex_unlock(&semaforoSWAP);
					if(mensaje->codigo_Operacion == ESCRIBIR_PAGINA_SWAP_OK){
						t_entradaPagina* entrada= mallocCommon(sizeof(t_entradaPagina));
						entrada->numeroPagina = i;
						entrada->presencia = 0;
						entrada->modificacion = 0;
						list_add(listaPaginas, (void*)entrada);
						log_info(logger,"Nueva pagina: %d correctamente almacenada en SWAP", i);
					}
				}

				tablaProceso->paginas = listaPaginas;
				// Agrego la tabla de paginas del proceso a la lista con todas las tablas de todos los procesos
				pthread_mutex_lock(&semaforoGlobal);
				list_add(listaTablasdePaginas, (void*)tablaProceso);
				pthread_mutex_unlock(&semaforoGlobal);
				//Informo a Kernel que se guardo la data en swap
				destruirPaquete(mensaje);
				mensaje = pedirPaquete(strdup("EL"),NUEVO_PROGRAMA_OK,3);
				enviar(socketNucleo, mensaje, NULL);
				log_info(logger,"Se almaceno el nuevo programa: %d Cantidad de Paginas: %d correctamente en SWAP", idProgramaNuevo,numeroPaginas);

			}else if(mensaje->codigo_Operacion==NUEVO_PROGRAMA_SWAP_SIN_ESPACIO){
				//Informo a Kernel que no hay lugar en SWAP para el nuevo proceso
				destruirPaquete(mensaje);
				mensaje = pedirPaquete(strdup("EL"),NO_HAY_ESPACIO_SWAP,3);
				enviar(socketNucleo, mensaje, NULL);
				log_error(logger,"No se dispone de lugar para el nuevo programa. Programa: %d Cantidad de Paginas: %d", idProgramaNuevo,numeroPaginas);
			}
			break;

		case FIN_PROGRAMA:

			memcpy(&idProgramaEliminar,mensaje->data,sizeof(int));

			pthread_mutex_lock(&semaforoGlobal);
			tablaFinPrograma = (t_tablaPaginas*)list_find(listaTablasdePaginas, (void*)buscarTablaPrograma);
			pthread_mutex_unlock(&semaforoGlobal);
			if(tablaFinPrograma!=NULL){
				pthread_mutex_lock(&semaforoGlobal);
				for(i=0; i<list_size(tablaFinPrograma->paginas); i++){
					t_entradaPagina* entradaFinPrograma;
					entradaFinPrograma = (t_entradaPagina*)list_get(tablaFinPrograma->paginas,i);
					if(entradaFinPrograma->presencia == 1){
						t_marco* marcoModificar;
						marcoModificar = (t_marco*)list_get(listaMarcos, entradaFinPrograma->frame);
						marcoModificar->asignado=false;
						list_replace(listaMarcos,marcoModificar->id,(void*)marcoModificar);
					}
				}
				pthread_mutex_unlock(&semaforoGlobal);
				bool datosListaPagina(t_datosCargaMemoria* dcm){
					return dcm->pid == tablaFinPrograma->PID;
				}
				pthread_mutex_lock(&semaforoGlobal);
				list_remove_by_condition(listaTablasdePaginas,(void*)buscarTablaPrograma);
				list_remove_by_condition(listaPaginasMemoria,(void*)datosListaPagina);
				finalizoProgramaEnTLB(idProgramaEliminar);
				pthread_mutex_unlock(&semaforoGlobal);
				log_info(logger, "Envio a SWAP finalizar el programa: %d", idProgramaEliminar);
				//avisar swap eliminar espacio en memoria para pid
				destruirPaquete(mensaje);
				mensaje = pedirPaquete(&idProgramaEliminar, FIN_PROGRAMA_SWAP,sizeof(int));
				pthread_mutex_lock(&semaforoSWAP);
				enviar(socketSWAP,mensaje, NULL);
				destruirPaquete(mensaje);
				mensaje = recibir(socketSWAP,NULL);
				pthread_mutex_unlock(&semaforoSWAP);
				if(mensaje->codigo_Operacion==FIN_PROGRAMA_SWAP_OK){


					destruirPaquete(mensaje);
					mensaje = pedirPaquete(strdup("OK"),FIN_PROGRAMA_OK,3);
					enviar(socketNucleo,mensaje,NULL);
					log_info(logger, "Finalizó el programa: %d, se 	eliminó su espacio de memoria", idProgramaEliminar);
				}
			}
			break;

		default:
			log_error(logger, "Recibi codigo invalido de Nucleo");
			destruirPaquete(mensaje);
			mensaje = pedirPaquete(strdup("FF"), ERROR_CODIGO_INVALIDO,3);
			enviar(socketNucleo,mensaje,NULL);
		}

	}


}
