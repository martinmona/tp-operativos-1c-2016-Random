/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include "consola.h"
#include "common_sockets.h"


int main(int argc, char* argv[]){

	if (argc != 2) {
			puts("Debe ingresar la ruta del archivo a ejecutar");
			return EXIT_FAILURE;
		}



		 t_config * config_cpu = config_create("../resources/config.cfg");
		 if( config_cpu == NULL )
		 {
		  puts("Error al abrir el archivo de configuraciones.");
		  abort();
		 }
		 t_configuracion configuracion;
		 // OBTENGO CONFIGURACION DEL CONFIG /
		 configuracion.ip = config_get_string_value(config_cpu, "IP_NUCLEO" );
		 configuracion.puerto = config_get_string_value(config_cpu, "PUERTO_NUCLEO");
		 free(config_cpu);

	puts("PROCESO CONSOLA");
	int socket_nucleo = conectar(configuracion.ip,configuracion.puerto);
	while(socket_nucleo == -1)
	{
		puts("Volviendo a intentar conectar con Nucleo en 5 segundos");
		sleep(5);
		socket_nucleo = conectar(configuracion.ip, configuracion.puerto);
	}
/*	char* mensaje = malloc(250);

	if (socket_nucleo > 0) {

		printf("Ingrese el mensaje a enviar, exit para salir \n");
		scanf("%s", mensaje);
		while(strcmp(mensaje, "exit")!=0){
			send(socket_nucleo, mensaje, 250, 0);
			 printf("Mensaje enviado. Ingrese el mensaje a enviar, exit para salir \n");
			 scanf("%s", mensaje);

		}

	}
*/


	//HANDSHAKE
	t_envio*mensaje = pedirPaquete(strdup("OK"),HANDSHAKE,3);
	enviar(socket_nucleo, mensaje, NULL );
	destruirPaquete(mensaje);
	mensaje = recibir(socket_nucleo,NULL);
	if (mensaje->codigo_Operacion == 1) {
		printf("CONECTADO CON EL NUCLEO, Socket: %d\n",socket_nucleo);
	}
	destruirPaquete(mensaje);
    int tamanioArchivo=tamArchivo(argv[1]);
    char*ansiSOP= malloc(tamanioArchivo);
	ansiSOP=obtenerContenidoAnSISOP(argv[1]);

	t_envio* msj= pedirPaquete(ansiSOP,ENVIAR_ANSISOP,tamanioArchivo);
	enviar(socket_nucleo,msj,NULL);
	destruirPaquete(msj);
	msj = recibir(socket_nucleo,NULL);
	if(msj->codigo_Operacion==FALTA_LUGAR_SWAP){
		printf("NO HAY LUGAR EN SWAP\n");
		return 0;
	}else if(msj->codigo_Operacion==PROGRAMA_ALMACENADO){
		printf("PROGRAMA GUARDADO EN SWAP Y EJECUTANDO\n");
	}


	// mostrar programaAnsisop

	bool salir = false;
	while(salir == false){

		mensaje= recibir(socket_nucleo,NULL);
		int operacion= mensaje->codigo_Operacion;
		switch(operacion){
		char* textoAImprimir;
		int valorAImprimir;
		case IMPRIMIR:
			memcpy(&valorAImprimir,mensaje->data,sizeof(int));
			printf("VALOR:%i \n",valorAImprimir);
			break;
		case IMPRIMIRTEXTO:

			textoAImprimir = malloc(mensaje->data_size);
			memcpy(textoAImprimir,mensaje->data,mensaje->data_size);
			printf("TEXTO: %s \n",textoAImprimir);
			free(textoAImprimir);
			break;


		case CERRARCONSOLA:
			printf("el procesamiento de su archivo ha finalizado\n");
			salir = true;
			destruirPaquete(mensaje);
			mensaje = pedirPaquete(strdup("OK"),CERRARCONSOLA,3);
			enviar(socket_nucleo,mensaje,NULL);


			break;

		case ERROR:
			printf("el archivo no se pudo procesar con exito\n");
			salir = true;
			destruirPaquete(mensaje);
			mensaje = pedirPaquete(strdup("OK"),CERRARCONSOLA,3);
			enviar(socket_nucleo,mensaje,NULL);
			break;


		destruirPaquete(mensaje);
		}

	}





	close(socket_nucleo);
	//free(mensaje);
	return EXIT_SUCCESS;
}
