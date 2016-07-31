/*
 * funcionesConsola.c
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */
#include "Swap.h"

void aplicarRetardo(){
	log_info(logger,"Retardo acceso a archivo");
	sleep(retAcceso/1000);
}
bool escribirPagina(int pid, int numeroDePagina, char*pagina) {
	log_info(logger,"escribiendo pagina %d de proceso %d",numeroDePagina,pid);
	espacio* nodoALeer= mallocCommon(sizeof(espacio));

	aplicarRetardo();

	bool encontrarNodoAsignado (espacio* ea){
		return ea->pid==pid && ea->numDePag == numeroDePagina;
	}
	nodoALeer = (espacio*)list_find(listaEspacios, (void*)encontrarNodoAsignado);

	int dondeEscribo = nodoALeer->inicio;
	int i;
	for (i = 0; i < tamPagina; ++i) {
		archivoMappeado[dondeEscribo+i] = pagina[i];
	}
	msync(archivoMappeado, tamArchivo, MS_SYNC);
	log_info(logger,"pagina escrita correctamente");
	//free(nodoALeer);
	return TRUE;
}

char* leerUnaPagina(int pid, int numeroDePagina) {
	log_info(logger,"leyendo pagina %d de proceso %d",numeroDePagina,pid);
	aplicarRetardo();
//	punteroAPaginaLeida=(&paginaLeida[0]);
	espacio* nodoALeer=mallocCommon(sizeof(espacio));
	bool encontrarNodoAsignado (espacio* ea){
		return ea->pid==pid && ea->numDePag == numeroDePagina;
	}
	nodoALeer = (espacio*)list_find(listaEspacios, (void*)encontrarNodoAsignado);
	char* punteroADevolver = mallocCommon(tamPagina);
	int i;
	for (i = 0; i < tamPagina; ++i) {
		punteroADevolver[i]=archivoMappeado[nodoALeer->inicio+i];
	}

	return (punteroADevolver);

}

void eliminarProceso(int pid) {
	int i;
	espacio*nodoAReventar=mallocCommon(sizeof(espacio));
	void marcarNoUsados(espacio* esp){
		if(esp->pid==pid){
			esp->bitMap=false;
		}
	}

	list_iterate(listaEspacios,(void*)marcarNoUsados);

	log_info(logger,"Proceso: %d  Eliminado", pid);

}

void reservarPaginas(int posicionNodoLibre, int pid, int cantidadDePaginas) {
	//Paso los nodos de lista de espacios libres a asignados
	int i;
	log_info(logger,"comienzo a asignar en la posicion %d",posicionNodoLibre);
	for (i = 0; i < cantidadDePaginas; ++i) {
		//Tomo el nodo de la lista de espacios libres
		espacio* espacioAAsignar= mallocCommon(sizeof(espacio));
		espacioAAsignar=(espacio*)list_get(listaEspacios,posicionNodoLibre+i);
		espacioAAsignar->numDePag=i;
		espacioAAsignar->pid=pid;
		espacioAAsignar->bitMap=true;
		//log_info(logger,"Reservado la pagina %d pid %d comienzo %d",espacioAAsignar->numDePag,espacioAAsignar->pid,espacioAAsignar->inicio);
	}

}



//compartar
bool estanOcupados(espacio * elem){
	return elem->bitMap;
}
bool estanLibres(espacio * elem){
	return !elem->bitMap;
}

void compactarEspacioLibre(){

	log_info(logger,"Comenzando compactacion");
	int i,j,comienzoEscribir;
	comienzoEscribir=0;
	espacio* espacioUno = malloc(sizeof(espacio));
	espacio* espacioDos = malloc(sizeof(espacio));
	t_list* listaEspaciosOcupados = list_create();
	listaEspaciosOcupados=list_filter(listaEspacios, (void*)estanOcupados);
	log_info(logger,"Lista espacios ocupados %d: ",list_size(listaEspaciosOcupados));

	for (i = 0; i < list_size(listaEspacios); ++i) {
		espacioDos= (espacio*)list_get(listaEspacios,i);
		if(i<list_size(listaEspaciosOcupados)){
			espacioUno = (espacio*)list_get(listaEspaciosOcupados,i);
			list_replace(listaEspacios,i,(void*)espacioUno);
			for (j = 0; j < tamPagina; ++j) {
				archivoMappeado[espacioDos->inicio+j] = archivoMappeado[espacioUno->inicio+j];
			}
			espacioDos->bitMap=true;
			espacioDos->numDePag=espacioUno->numDePag;
			espacioDos->pid=espacioUno->pid;

		}else{
			espacioDos->bitMap=false;
		}
		list_replace(listaEspacios,i,(void*)espacioDos);

	}

	msync(archivoMappeado, tamArchivo, MS_SYNC);
	sleep(retCompactacion/1000);
	log_info(logger,"Compactacion completada. Retardo de: %d segundos", 1000 * retCompactacion);
}

// da la posision inicial de la pagina donde se puede empezar a guardar, sino devuelve fragmentacion externa
int paginasContiguas(int cantPaginas){
	espacio* espacioUno= mallocCommon(sizeof(espacio));
	espacio* espacioSiguiente= mallocCommon(sizeof(espacio));
	int posicion=0;
	int contadorDePaginasSeguidas=1;
	int tamListaEsp=list_size(listaEspacios);
	if(cantPaginas <= tamListaEsp){
		while(posicion < tamListaEsp-1 && (cantPaginas>contadorDePaginasSeguidas)){
			espacioUno=(espacio*)list_get(listaEspacios,posicion);
			espacioSiguiente=(espacio*)list_get(listaEspacios,posicion+1);

			if(espacioUno->bitMap ==false && espacioSiguiente->bitMap==false){
				contadorDePaginasSeguidas++;
			}
			else{
				contadorDePaginasSeguidas=1;
			}
			posicion++;
			//free(espacio);
			//free(espacioSiguiente);
		}

	}
	log_info(logger,"posicion pags cont %d  cantidad pags seguidas %d",posicion,contadorDePaginasSeguidas);
	if(cantPaginas==contadorDePaginasSeguidas){
		return posicion-contadorDePaginasSeguidas+1;
	}
	else{
		return FRAGMENTACION_EXTERNA;
	}
}


//verificar si hay espacio
bool hayEspacio(espacio* esp){
	return esp->bitMap==false;
}

//recibir nuevo programa

bool recibirNuevoPrograma(int pid, int cantidadPaginas){
	int pagComienzo=0;
	if(list_count_satisfying(listaEspacios,(void*)hayEspacio)>cantidadPaginas){
		log_info(logger,"Hay espacio para almacenar el nuevo programa");
		pagComienzo=  paginasContiguas(cantidadPaginas);
		log_info(logger,"Direccion donde escribir %d", pagComienzo);
		if(pagComienzo == FAIL){
			compactarEspacioLibre();
			pagComienzo =  paginasContiguas(cantidadPaginas);
		}

		reservarPaginas(pagComienzo,pid,cantidadPaginas);
		log_info(logger,"Proceso: %d Agregado correctamente", pid);
		return TRUE;
	}
	log_error(logger,"No hay espacio suficiente para el nuevo programa");
	return FALSE;
}

//creo listas
void crearListas() {
	listaEspacios = list_create();
}

//Cargo la lista de espaciolibre
void cargarEspaciosLibres(){
	int i;
	for (i = 0; i < paginas; ++i) {
		espacio * nuevoEspacioLibre = mallocCommon(sizeof(espacio));
		nuevoEspacioLibre->inicio = i*tamPagina;
		nuevoEspacioLibre->tamanio= tamPagina;
		nuevoEspacioLibre->bitMap=false;
		list_add(listaEspacios , (void*)nuevoEspacioLibre);

	}

}


void* mappearArchivo(char* filename) {
	char *addr;
	int fd;
	struct stat sb;
	size_t length;
	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	length = sb.st_size;
	lseek(fd, tamArchivo-1, SEEK_SET);
	write(fd, "", 1);
	addr = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	return addr;
}

//Genero mi archivo de Swap y lo devuelvo mappeado en memoria
char* crearArchivo(char* tamanio,char*nombre){
	/*char* paraSistema=string_new();
	string_append(&paraSistema,"dd if=/dev/zero of=");
	string_append(&paraSistema,nombre);
	string_append(&paraSistema," bs=");
	string_append(&paraSistema,tamanio);
	string_append(&paraSistema," count=1");
	system(paraSistema);
	free(paraSistema);*/
	char*archivo=mappearArchivo(nombre);
	return archivo ;

}


void setearValores(t_config * archivoConfig) {
		umcPort = config_get_string_value(archivoConfig, "PUERTO_UMC");
		nombreSwap = config_get_string_value(archivoConfig, "NOMBRE_SWAP");
		paginas = config_get_int_value(archivoConfig, "CANTIDAD_PAGINAS");
		tamPagina = config_get_int_value(archivoConfig, "TAMANO_PAGINA");
		retCompactacion = config_get_int_value(archivoConfig, "RETARDO_COMPACTACION");
		tamArchivo=paginas*tamPagina;
		log_info(logger,"cant pags %d tam pags %d tam archivo %d",paginas,tamPagina,tamArchivo);
		retAcceso = config_get_int_value(archivoConfig, "RETARDO_ACCESO");
}

int verificarExistenciaDeArchivo(char* rutaArchivoConfig) {
	FILE * archivoConfig = fopen(rutaArchivoConfig, "r+");
	if (archivoConfig!=NULL){
		fclose(archivoConfig);
		return 1;
	}
	return FAIL;
}



void leerArchivoDeConfiguracion( char * direccionArchivo) {
	logger = log_create("../resources/log.txt", "SWAP", true, LOG_LEVEL_DEBUG);
	log_info(logger, "Log creado");
	t_config* archivoDeConfiguracion;
	char*configPath = direccionArchivo;

	if (verificarExistenciaDeArchivo(configPath) == FAIL)
		puts("[ERROR] Archivo de configuracion no encontrado");


	archivoDeConfiguracion = config_create(configPath);
	setearValores(archivoDeConfiguracion);

}


void recibirComandos(int socketUMC) {
	t_envio* mensaje = mallocCommon(sizeof(t_envio));
	char* data = mallocCommon(tamPagina);
	void errorRecibir(){
		log_error(logger,"Error al recibir mensaje de UMC. Cerrando conexion");
		close(socketUMC);
		mensaje=NULL;
	}

      mensaje=recibir(socketUMC,NULL);
      		if (mensaje->codigo_Operacion == 1) {
      			int tamanioPagUMC;
      			memcpy(&tamanioPagUMC,mensaje->data,sizeof(int));
      			if(tamanioPagUMC!=tamPagina) log_warning(logger,"TAMANIO DE PAGINAS DISTINTO POR CONFIGURACION. TAMANO UMC: %d - TAMANO SWAP: %d",tamPagina,tamanioPagUMC);
      			mensaje = pedirPaquete(strdup("OK"),HANDSHAKE,3);
      			enviar(socketUMC, mensaje, NULL );
      			printf("CONECTADO CON LA UMC, Socket: %d\n",socketUMC);
      		}

	mensaje= recibir(socketUMC, (void*)errorRecibir);

	while (mensaje!=NULL) {
		int pid, numPagina,cantPaginas;
		int tipoMensaje = mensaje->codigo_Operacion;

		switch (tipoMensaje) {
		case SOLICITUD_DE_INCIO: {

			memcpy(&pid, mensaje->data,sizeof(int));
			memcpy(&cantPaginas, mensaje->data+sizeof(int),sizeof(int));
			log_info(logger,"Solicitud de nuevo programa. PID: %d CANTIDAD PAGINAS: %d", pid, cantPaginas);
			if(recibirNuevoPrograma(pid, cantPaginas)){

				t_envio* msj=pedirPaquete(strdup("OK"),HAYLUGAR,3);
				enviar(socketUMC, msj,NULL);
				destruirPaquete(msj);
			}
			else{

				t_envio* msj=pedirPaquete(strdup("FAIL"),NOHAYLUGAR,5);
				enviar(socketUMC, msj,NULL);
				destruirPaquete(msj);

			}

			break;
		}
		case SOLICITUD_LEER: {

			memcpy(&pid, mensaje->data,sizeof(int));
			memcpy(&numPagina, mensaje->data+sizeof(int),sizeof(int));
			log_info(logger,"Solicitud de leer Pagina: %d Proceso: %d", numPagina,pid);
			char* leido = leerUnaPagina(pid,numPagina);
			//log_info(logger,"largo leido: %d", string_length(leido));
			t_envio* msj=pedirPaquete(leido,DATOLEIDO,tamPagina);
			enviar(socketUMC, msj,NULL);
			destruirPaquete(msj);
			log_info(logger,"Lectura correcta Pagina: %d Proceso: %d", numPagina,pid);
			//free(leido);
			break;
		}

		case SOLICITUD_ESCRITURA: {
			memcpy(&pid, mensaje->data,sizeof(int));
			memcpy(&numPagina, mensaje->data+sizeof(int),sizeof(int));
			memcpy(data, mensaje->data+sizeof(int)+sizeof(int),tamPagina);
			log_info(logger,"Solicitud de escribir Pagina: %d Proceso: %d", numPagina,pid);

			if(escribirPagina(pid,numPagina,data)){
				t_envio* msj=mallocCommon(sizeof(t_envio));
				msj=pedirPaquete(strdup("OK"),ESCRIBIOPAGINA,3);
				enviar(socketUMC, msj,NULL);
				destruirPaquete(msj);

			}
			log_info(logger,"Escritura correcta Pagina: %d Proceso: %d", numPagina,pid);
			break;
		}

		case FIN_PROCESO: {

			memcpy(&pid, mensaje->data,sizeof(int));
			log_info(logger,"Solicitud fin de Proceso: %d", pid);
			eliminarProceso(pid);
			t_envio* msj=mallocCommon(sizeof(t_envio));
			msj= pedirPaquete(strdup("OK"),ELIMINOPROCESO,3);
			enviar(socketUMC, msj,NULL);
			destruirPaquete(msj);

		}

		}

		mensaje= recibir(socketUMC, (void*)errorRecibir);

	}


}


