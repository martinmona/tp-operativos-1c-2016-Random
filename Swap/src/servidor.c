/*
 ============================================================================
 Name        : servidor.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int iniciarServidor(void) {
	struct sockaddr_in pa;
	pa.sin_family = AF_INET;
	pa.sin_addr.s_addr = INADDR_ANY;
	pa.sin_port = htons(7000);
	int servidor = socket(AF_INET, SOCK_STREAM, 0);
	//int activado = 1;
	//setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
	if (bind(servidor, (void*) &pa, sizeof(pa)) != 0) {
		perror("Falló el bind");
		return 1;
	}
	printf("Estoy escuchando\n");
	listen(servidor, 100);
	//------------------------------
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int cliente = accept(servidor,(struct sockaddr *) &addr,	&addrlen);
	printf("Recibí una conexión en %d!!\n", cliente);
	int handshake = 5;
	recv(cliente, &handshake,sizeof handshake, 0);
	send(cliente, &handshake,sizeof handshake, 0);
	if(handshake == 4){
		printf("CONECTADO CON LA UMC, Socket: %d\n",cliente);
	}

	return cliente;
}
