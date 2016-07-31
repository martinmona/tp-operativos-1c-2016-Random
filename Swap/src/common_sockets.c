#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/error.h>
#include <errno.h>
#include "common_sockets.h"

void* mallocCommon(int size) {
	void* return_memory = NULL;
	if (size != 0) {
		while (return_memory == NULL) {
			return_memory = malloc(size);
		}
	}
	return return_memory;
}



void aniadirAlPaquete(t_envio* paquete, void* datosExtra,
		int sizeExtra)
{
	void* dataClumpExtra = mallocCommon(paquete->data_size + sizeExtra);
	memcpy(dataClumpExtra, paquete->data, paquete->data_size);
	memcpy(dataClumpExtra + paquete->data_size, datosExtra, sizeExtra);
	free(paquete->data);
	paquete->data_size += sizeExtra;
	paquete->data = dataClumpExtra;
}

t_envio* pedirPaquete(void* data, int codigoOp,
		int size)
{
	t_envio* paquete = mallocCommon(sizeof(t_envio));
	paquete->codigo_Operacion = codigoOp;
	paquete->data_size = size;
	paquete->data = mallocCommon(size);
	memcpy(paquete->data, data, size);
	return paquete;
}

void destruirPaquete(t_envio* paquete)
{
	free(paquete->data);
	free(paquete);
}

static char* serializar(t_envio* unPaquete)
{
	char* stream = mallocCommon((packet_header) + unPaquete->data_size);
	memcpy(stream, unPaquete, packet_header);
	memcpy(stream + packet_header, unPaquete->data, unPaquete->data_size);
	return stream;
}

static t_envio* deserializarHeader(char* buffer)
{
	t_envio* unPaquete = mallocCommon(sizeof(t_envio));
	memcpy(&unPaquete->codigo_Operacion, buffer, sizeof(int));
	memcpy(&unPaquete->data_size, buffer + sizeof(int),
			sizeof(int));
	return unPaquete;
}

static void deserializarData(char* buffer, t_envio* unPaquete)
{
	unPaquete->data = mallocCommon(unPaquete->data_size);
	memcpy(unPaquete->data, buffer, unPaquete->data_size);
}

/**  @NAME: common_setup
 *	 @DESC: Retorna un puntero a una addrInfo totalmente lista (Si, es una lista y veo el pun)
 *	 para usar, dado una IP y un Host cualesquiera.
 *	 No deberia ser usado fuera de common_sockets.
 *	 @RETURN: Devuelve NULL si falla.
 */

struct addrinfo* common_setup(char *IP, char* Port)
{
	struct addrinfo hints;
	struct addrinfo* serverInfo;
	int16_t error;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (!strcmp(IP, "localhost"))
	{
		hints.ai_flags = AI_PASSIVE;
		error = getaddrinfo(NULL, Port, &hints, &serverInfo);
	}
	else
		error = getaddrinfo(IP, Port, &hints, &serverInfo);
	if (error)
	{
		error_show("Problema con el getaddrinfo()!: %s", gai_strerror(error));
		return NULL ;
	}
	return serverInfo;
}

/**	@NAME: connect_to
 * 	@DESC: Intenta establecer conexion con el servidor que deberia estar escuchando. Controla errores, vuelve cuando conecta
 * 	@RETURN: Devuelve EXIT_FAILURE (1) si fallo la conexion, en otro caso devuelve el socket.
 *
 */
int conectar(char *IP, char* Port)
{
	struct addrinfo* serverInfo = common_setup(IP, Port);
	if (serverInfo == NULL )
	{
		return -1;
	}
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (serverSocket == -1)
	{
		error_show(
				"\n No se pudo disponer de un socket al llamar connect_to, error: %d, man errno o internet!\n",
				errno);
		return -1;
	}
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			== -1)
	{
		puts("\n");
		error_show(
				"No se pudo conectar con el proceso que hace de servidor, error: %d\n",
				errno);
		close(serverSocket);
		return -1;
	}
	//char s[INET6_ADDRSTRLEN];
	//inet_ntop(serverInfo->ai_family, get_in_addr((struct sockaddr *)serverInfo->ai_addr),s, sizeof s);
	//printf("Conectando con %s\n", s);
	freeaddrinfo(serverInfo);
	return serverSocket;
}

/**
 * @NAME: envio
 * @DESC: Intenta mandar un paquete por el socket.
 * Devuelve EXIT_SUCCESS.
 */
void enviar(int socket, t_envio* paquete, void (*executeIfError)())
{
	char* buffer;
	int totalLength;
	int quantitySend;
	int enviando = 1;
	int offset = 0;
	buffer = serializar(paquete);
	totalLength = paquete->data_size + packet_header;
	while (enviando)
	{
		quantitySend = send(socket, buffer + offset, totalLength - offset, 0);
		if (quantitySend == -1)
		{
			error_show("Error al enviar, error: %d", errno);
			executeIfError();
		}
		if (quantitySend < totalLength)
		{
			totalLength = totalLength - quantitySend;
			offset = quantitySend;
			//sigo enviando
		}
		else
			enviando = 0;
	}
	free(buffer);
}

/** @NAME recibir
 *  @DESC: Recibe paquetes a traves de fd, no implementa select ni poll. Si hay error ejecuta executeIfError
 *  @RETURN: Retorna el paquete recibido
 */
t_envio* recibir(int fd, void (*executeIfError)())
{
	char *buffer = mallocCommon(packet_header);
	int buffer_size = recv(fd, buffer, packet_header, MSG_WAITALL);
	if (buffer_size < 0)
	{
		error_show("Error al recibir paquete - %d", errno);
		executeIfError();
		return NULL ;
	}
	if (buffer_size == 0)
	{
		error_show("Conexion cerrada en socket: %d", fd);
		executeIfError();
		return NULL ;
	}
	t_envio* dataToReceive = deserializarHeader(buffer);
	free(buffer);

	buffer = mallocCommon(dataToReceive->data_size);
	buffer_size = recv(fd, buffer, dataToReceive->data_size, MSG_WAITALL);
	if (buffer_size < 0)
	{
		error_show("Error al recibir paquete - %d", errno);
		executeIfError();
		return NULL ;
	}
	if (buffer_size == 0)
	{
		error_show("Conexion cerrada en socket: %d", fd);
		executeIfError();
		return NULL ;
	}
	deserializarData(buffer, dataToReceive);
	free(buffer);
	return dataToReceive;
}



int iniciarEscucha(char* IP,char* Port)
{
	int yes=1;
	struct addrinfo* serverInfo = common_setup(IP, Port);
	if (serverInfo == NULL )
		return -1;
	int socketEscucha;
	socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	if (setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}
	bind(socketEscucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return socketEscucha;
}



