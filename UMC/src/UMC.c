/*
 ============================================================================
 Name        : UMC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "UMC.h"




int main(void) {
	puts("!!!PROCESO UMC!!!");
	//Cargo archivo de config y Log
	cargarArchivoConfig();
	inicializar();

	socketSWAP = conectar(miConfig.ipSWAP, miConfig.puertoSWAP);

	if(socketSWAP == -1){
		while(socketSWAP == -1)
			{
				log_error(logger, "Volviendo a intentar conectar con SWAP en 5 segundos");
				sleep(5);
				socketSWAP = conectar(miConfig.ipSWAP, miConfig.puertoSWAP);
			}
	}
	log_info(logger,"Esperando paquete swap");
	t_envio* mensaje = pedirPaquete(&miConfig.tamanioMarco,HANDSHAKE,sizeof(int));
	log_info(logger,"paquete pedido swap");
	enviar(socketSWAP, mensaje, NULL);
	mensaje = recibir(socketSWAP, NULL );
	if (mensaje->codigo_Operacion != SWAP_OK) {
		log_error(logger,"Error handshake con swap");
		destruirPaquete(mensaje);
	}

	log_info(logger, "Conectado con SWAP en Socket: %d", socketSWAP);
	printf("Ya puede ingresar comandos\n");

	//CONEXION
	if ((socketEscucha = iniciarEscucha("localhost", miConfig.puertoEscucha))
			< 0) {
		//ERROR
		log_error(logger, "FUNCION SOCKET");
	}
	if (listen(socketEscucha, 10) < 0) {
		//ERROR
		log_error(logger,"FUNCION LISTEN");
	}

	pthread_t hiloConexiones, hiloConsola;
	pthread_create(&hiloConexiones, NULL, (void*) escucharConexion, NULL);
	pthread_create(&hiloConsola, NULL, (void*) consolaUMC, NULL);
	pthread_join(hiloConexiones,NULL);
	pthread_join(hiloConsola,NULL);
	pthread_mutex_destroy(&semaforoSWAP);
	pthread_mutex_destroy(&semaforoGlobal);

	return EXIT_SUCCESS;
}

