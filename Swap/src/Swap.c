/*
 ============================================================================
 Name        : Swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "servidor.h"
#include "Swap.h"

int main(void) {
	puts("PROCESO SWAP"); /* prints !!!Hello World!!! */

	leerArchivoDeConfiguracion("../resources/swapconfig.cfg");
	archivoMappeado=crearArchivo(tamArchivo,nombreSwap);
	log_info(logger,"Archivo de configuracion cargado y archivo de swap creado");
	crearListas();
	cargarEspaciosLibres();

	//int socket = iniciarServidor();

/*
		char* buffer = malloc(250);
		while (1) {
			int bytesRecibidos = recv(socket, buffer, 250, 0);
			if (bytesRecibidos <= 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}
			buffer[bytesRecibidos] = '\0';
			printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
		}
		free(buffer);

		leerArchivoDeConfiguracion("../swapconfig");

	    //Creo mi archivo de swap
	    archivoMappeado=crearArchivo(tamArchivo,nombreSwap);
	    */

		int socket;
		//CONEXION (cambiar el 7000 por el puerto en el archivo de configuracion)
		if ((socket = iniciarEscucha("localhost",umcPort))
				< 0) {
			//ERROR
			//log_error(logger, "FUNCION SOCKET");
		}
		if (listen(socket, 10) < 0) {
			//ERROR
			//log_error(logger,"FUNCION LISTEN");
		}

		//Esperando UMC
		listen(socket,100);
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);
		int socketUMC = accept(socket,(struct sockaddr *) &addr,	&addrlen);
		log_info(logger,"Recibí una conexión en %d\n", socketUMC);



		//esperar mensaje de UMC
		recibirComandos( socketUMC);

		return EXIT_SUCCESS;
}
