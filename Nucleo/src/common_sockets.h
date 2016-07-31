#ifndef COMMON_SOCKETS_H_
#define COMMON_SOCKETS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


typedef struct {
	int codigo_Operacion;
	int data_size;
	void* data;

} t_envio;

#define packet_header (sizeof(int) + sizeof(int))

void destruirPaquete(t_envio*);
t_envio* pedirPaquete(void* data,int codigoOp, int size);
struct addrinfo* common_setup(char *IP, char* Port);
int conectar(char *IP, char* Port);
void enviar(int socket, t_envio* paquete,void (*executeIfError)());
t_envio* recibir(int fd, void (*executeIfError)());
int iniciarEscucha(char* IP,char* Port);
void aniadirAlPaquete(t_envio* paquete, void* datosExtra, int sizeExtra);

#endif /* COMMON_SOCKETS_H_ */
