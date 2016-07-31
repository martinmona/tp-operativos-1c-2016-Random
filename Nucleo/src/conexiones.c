/*
 * conexiones.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#include "conexiones.h"

#define BACKLOG 10


void escucharConexionCPU() {
	log_info(logger,"escuchando CPUs");
	while (1) {
		FD_ZERO(&descriptoresLecturaCPU);
		FD_SET(socketEscuchaCPU, &descriptoresLecturaCPU);

		socketActivoCPU = select(socketEscuchaCPU + 1, &descriptoresLecturaCPU, NULL,
				NULL, NULL );

		if (socketActivoCPU < 0) {
			//error
			log_error(logger, "FUNCION SELECT()");
			pthread_exit(NULL );
		}
		if (FD_ISSET(socketEscuchaCPU,&descriptoresLecturaCPU)) {
			atenderConexionCPU(socketEscuchaCPU); //aceptar la conexion
		}

	}
}

void atenderConexionCPU(int listener) {
	int socket;
	pthread_t threadHilo;
	int socketHilo;
	t_envio *mensaje = mallocCommon(sizeof(t_envio));
	struct sockaddr_in dirClienteEntrante;
	int tam = sizeof(struct sockaddr_in);

	socket = accept(listener, (struct sockaddr *) &dirClienteEntrante,
			(void*) &tam);
	mensaje = recibir(socket, NULL );

	if (mensaje->codigo_Operacion == CPU_Nucleo_Handshake) {				// con CPU
		socketHilo = socket;
		mensaje = pedirPaquete(&config.QUANTUM,CPU_Nucleo_Ok,sizeof(int));
		aniadirAlPaquete(mensaje,&config.QUANTUM_SLEEP,sizeof(int));
		enviar(socket, mensaje, NULL ); //CONEXION CREADA CON EXITO
		log_info(logger, "SE ACEPTO UN NUEVO CPU");
		pthread_create(&threadHilo, NULL, &hiloCPU, &socketHilo);
	}
}

void escucharConexionConsola() {
	log_info(logger,"escuchando Consolas");
	while (1) {
		FD_ZERO(&descriptoresLecturaConsola);
		FD_SET(socketEscuchaConsola, &descriptoresLecturaConsola);

		socketActivoConsola = select(socketEscuchaConsola + 1, &descriptoresLecturaConsola, NULL,
				NULL, NULL );

		if (socketActivoConsola < 0) {
			//error
			log_error(logger, "FUNCION SELECT()");
			pthread_exit(NULL );
		}
		if (FD_ISSET(socketEscuchaConsola,&descriptoresLecturaConsola)) {
			atenderConexionConsola(socketEscuchaConsola); //aceptar la conexion
		}

	}
}

void atenderConexionConsola(int listener) {
	int socket;
	pthread_t threadHilo;
	int socketHilo;
	t_envio *mensaje = mallocCommon(sizeof(t_envio));
	struct sockaddr_in dirClienteEntrante;
	int tam = sizeof(struct sockaddr_in);

	socket = accept(listener, (struct sockaddr *) &dirClienteEntrante,
			(void*) &tam);
	mensaje = recibir(socket, NULL );

	if (mensaje->codigo_Operacion == Consola_Nucleo_Handshake) {				// con Consola
		socketHilo = socket;
		mensaje = pedirPaquete(strdup("OK"),1,sizeof(int));
		enviar(socket, mensaje, NULL ); //CONEXION CREADA CON EXITO
		log_info(logger, "SE ACEPTO UN NUEVO CONSOLA");
		pthread_create(&threadHilo, NULL, &hiloConsola, &socketHilo);
	}
}
