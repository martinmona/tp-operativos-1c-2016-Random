/*
 * conexiones.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#include "conexiones.h"


#define BACKLOG 10


void escucharConexion() {

	while (1) {
		FD_ZERO(&descriptoresLectura);
		FD_SET(socketEscucha, &descriptoresLectura);

		socketActivo = select(socketEscucha + 1, &descriptoresLectura, NULL,
				NULL, NULL );

		if (socketActivo < 0) {
			//error
			log_error(logger, "FUNCION SELECT()");
			pthread_exit(NULL );
		}
		if (FD_ISSET(socketEscucha,&descriptoresLectura)) {
			atenderConexion(socketEscucha); //aceptar la conexion
		}

	}
}

void atenderConexion(int listener) {
	int socket;
	pthread_t threadHilo;
	int socketKernel;
	int socketHilo;
	t_envio *mensaje = mallocCommon(sizeof(t_envio));
	struct sockaddr_in dirClienteEntrante;
	int tam = sizeof(struct sockaddr_in);

	socket = accept(listener, (struct sockaddr *) &dirClienteEntrante,
			(void*) &tam);
	mensaje = recibir(socket, NULL );

	if (mensaje->codigo_Operacion == NUEVO_KERNEL) { 		// con KERNEL
		socketKernel = socket;
		mensaje = pedirPaquete(&miConfig.tamanioMarco,NUEVO_KERNEL_OK,sizeof(int));
		enviar(socketKernel, mensaje, NULL );
		log_info(logger, "SE ACEPTO AL KERNEL ");
		pthread_create(&threadHilo, NULL, &hiloKernel, &socketKernel);
	}
	if (mensaje->codigo_Operacion == NUEVA_CPU) {				// con CPU
		socketHilo = socket;
		mensaje = pedirPaquete(&miConfig.tamanioMarco,NUEVA_CPU_OK,sizeof(int));
		enviar(socket, mensaje, NULL ); //CONEXION CREADA CON EXITO
		log_info(logger, "SE ACEPTO UN NUEVO CPU");
		pthread_create(&threadHilo, NULL, &hiloCPU, &socketHilo);
	}
}
