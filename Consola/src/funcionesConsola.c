/*
 * funcionesConsola.c
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include "consola.h"
#include <string.h>



void empaquetarYenviarArchivo(t_archivo* archivo){

             char* contenido= malloc(sizeof(int)*2 + archivo->tamArchivo + strlen(archivo->ansiSOP)+1);

              t_contenido* paquete =malloc(sizeof(t_contenido));

              int offset=0;
              int tmp_size=sizeof(int)*2;

               memcpy(contenido, &(archivo->protocolo), tmp_size);

               offset+= tmp_size;

             memcpy(contenido + offset, &(archivo->tamArchivo), tmp_size=archivo->tamArchivo);

               offset+= tmp_size;

               tmp_size=strlen(archivo->ansiSOP);
               memcpy(contenido + offset, &(archivo->ansiSOP), tmp_size+1);

          paquete->tamanio=   offset+ tmp_size;
          paquete->contenido= contenido;


           send(socket_nucleo,paquete,sizeof(paquete),0);

        free(contenido);
        free(paquete);
}





int tamArchivo(char * direccionArchivo) {

	FILE * fp;

	int tamanioArchivo;
     fp = fopen(direccionArchivo, "r");
	if (fp == NULL)
		puts("Error al abrir archivo");
	else {
		fseek(fp, 0, SEEK_END);
		tamanioArchivo = ftell(fp);
		fclose(fp);
		printf("El tamaño de %s es: %d bytes.\n\n", direccionArchivo, tamanioArchivo);
	}

	return tamanioArchivo;
}

 char* obtenerContenidoAnSISOP(char * direccionArchivo) {

	 FILE * fp;
	 fp = fopen(direccionArchivo, "r");
	 if (fp == NULL){
		 puts("Error al abrir archivo");
	 }else {

		 fseek(fp, 0, SEEK_END);
		 long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = (char *) malloc(fsize + 1);
		if(buffer==NULL){
			printf("no se pudo reservar memoria para el archivo");
			return "";
		}
		fread(buffer, fsize, 1, fp);
		buffer[fsize] = '\0';
		fclose(fp);
		printf("%s", buffer);
		return buffer;
	 }
	 return "";
 }


