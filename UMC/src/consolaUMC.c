/*
 * consolaUMC.c
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#include "FuncionesUMC.h"
#include "consolaUMC.h"

const char* allCommands[] =
{
	"ayuda",
	"limpiar",
	"retardo",
	"dump",
	"flush",
	"salir"
};


void consolaUMC(){
	int command;
	int i;


	printf("Cantidad de Marcos en Memoria es: %d, con un máximo de %d por proceso\n",miConfig.cantidadMarcos, miConfig.maximoMarcoXProceso);


	do{
		fflush(stdin);		//limpia el buffer del teclado
		printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
		// leeComando lee la palabra y me devuelve un comando del enum
		command = leeComando();
		}
	while (command ==-1);

	while(command != salir) // Itero por siempre
	{
		switch (command)
		{
			case ayuda:
			{
				printf("\n Ayuda: \n Comandos disponibles: \n");
				for (i = 0; i <= salir; i++)
				{
					printf("%02d:\t%s\n", i + 1, allCommands[i]);
				}

				break;
			}
			case limpiar:
			{
				system("clear");
				break;

			}
			case retardos:
			{
				printf("Ingrese nuevo retardo en milisegundos \n");
				int nuevoRetardo;
				do{
					scanf("%d", &nuevoRetardo);
				}while(nuevoRetardo < 0);

				miConfig.retardoMemoria = nuevoRetardo;
				log_info(logger,"Nuevo retardo configurado: %d milisegundos",miConfig.retardoMemoria);

				break;
			}
			case dump:
			{
				int pid;
				printf("Ingrese PID del prceso, -1 para elegir todos\n");
				scanf("%d",&pid);
				//Generar reporte
				if(pid == -1){
					//Hago de todos los procesos
					int i;
					for (i = 0; i < list_size(listaTablasdePaginas); ++i) {
						t_tablaPaginas* tablaTmp = mallocCommon(sizeof(t_tablaPaginas));
						tablaTmp = (t_tablaPaginas*)list_get(listaTablasdePaginas,i);
						int j;
						for (j = 0; j < list_size(tablaTmp->paginas); ++j) {
							t_entradaPagina* entradaTmp = mallocCommon(sizeof(t_entradaPagina));
							entradaTmp = (t_entradaPagina*)list_get(tablaTmp->paginas,i);
							if(entradaTmp->presencia==1){
								t_marco* marco = mallocCommon(sizeof(t_marco));
								marco = (t_marco*)list_get(listaMarcos,entradaTmp->frame);
								char* contenido = mallocCommon(miConfig.tamanioMarco);
								memcpy(contenido, marco->comienzo,miConfig.tamanioMarco);
								printf("Proceso: %d Pagina: %d Marco: %d Contenido: %s \n", tablaTmp->PID,entradaTmp->numeroPagina,marco->id, contenido);
							}
						}
					}
				}else{
					//Busco los datos del proceso PID
					t_tablaPaginas* tablaTmp = mallocCommon(sizeof(t_tablaPaginas));
					bool buscarTablaPrograma(t_tablaPaginas* unaTablaDePags){
						return unaTablaDePags->PID==pid;
					}
					if(list_any_satisfy(listaTablasdePaginas, (void*)buscarTablaPrograma)){

						tablaTmp = (t_tablaPaginas*)list_find(listaTablasdePaginas, (void*)buscarTablaPrograma);
						int j;
						for (j = 0; j < list_size(tablaTmp->paginas); ++j) {
							t_entradaPagina* entradaTmp = mallocCommon(sizeof(t_entradaPagina));
							entradaTmp = (t_entradaPagina*)list_get(tablaTmp->paginas,j);
							if(entradaTmp->presencia==1){
								t_marco* marco = mallocCommon(sizeof(t_marco));
								marco = (t_marco*)list_get(listaMarcos,entradaTmp->frame);
								char* contenido = mallocCommon(miConfig.tamanioMarco+1);
								memcpy(contenido, marco->comienzo,miConfig.tamanioMarco);
								printf("Proceso: %d Pagina: %d Marco: %d Contenido: %s \n", tablaTmp->PID,entradaTmp->numeroPagina,marco->id, contenido);

							}
						}
					}else{
						printf("ERROR PID INVAILDO\n");
					}
				}

				break;
			}
			case flush:
			{
				int tlbOMemory;
				int pidFlush;
				printf("1. TLB \n2. Memory\n");
				scanf("%d",&tlbOMemory);
				if(tlbOMemory==1){
					//Limpio TLB
					list_clean(listaTLB);
					log_info(logger,"TLB limpiada");
				}else if(tlbOMemory ==2){

					printf("Ingrese PID de proceso:\n");
					scanf("%d",&pidFlush);
					//busco tabla de paginas de proceso
					t_tablaPaginas* tablaTmp = mallocCommon(sizeof(t_tablaPaginas));
					bool buscarTablaPrograma(t_tablaPaginas* unaTablaDePags){
						return unaTablaDePags->PID==pidFlush;
					}
					if(list_any_satisfy(listaTablasdePaginas, (void*)buscarTablaPrograma)){
						tablaTmp = (t_tablaPaginas*)list_find(listaTablasdePaginas, (void*)buscarTablaPrograma);
						int i;
						for (i = 0; i < list_size(tablaTmp->paginas); ++i) {
							t_entradaPagina* entradaTmp = mallocCommon(sizeof(t_entradaPagina));
							entradaTmp = (t_entradaPagina*)list_get(tablaTmp->paginas,i);
							if(entradaTmp->presencia==1){
								entradaTmp->modificacion=true;
							}
						}
						log_info(logger,"Las paginas en memoria del proceso: %d fueron marcadas como modificadas", tablaTmp->PID);
					}else{
						printf("ERROR PID INVAILDO\n");
					}
				}

				break;
			}
			case salir:
			{
				command=-1;
				printf("saliendo\n");


				break;
			}
			case enter:
			{
				// no haga nada, que se refleje el enter en la consola
				break;
			}
		}

			do{
					fflush(stdin);	//limpia el buffer del teclado
					printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
					command = leeComando();
					}// leeComando lee la palabra y me devuelve un comando del enum
			while (command ==-1);


	}
	printf("salio\n");
	exit(1);

}















//Funcion que lee el comando y lo busca entre los reconocidos
int leeComando(void)
{
	int c, i = 0;
	char *palabra=(char*)mallocCommon(sizeof(char)*40);
	if (palabra == NULL) puts("ERROR mallocCommon");
	while ((c = getchar()) != '\n') //leo hasta identificar enter
	{
		if (i > 40)
		{
			// si me pasé del largo de comando permitido devuelvo -1 para que el autómata
			// identifique que no se ha ingresado un comando permitdo
			printf("No se pudo encontrar el comando especificado\n");
			printf("Ingrese el comando deseado o ayuda para conocer los comandos posibles\n");
			return -1;
		}
		else
		{
			// Lo paso a minuscula y concateno
			palabra[i] = (char)tolower(c);
		}
		i++;
	}
	if (!i)
	{
		return enter;
	}
	palabra[i] = '\0';
	//si estoy aca es porque detecte un enter! Entonces mi palabra deberia estar lista para comparar
	i = 0;
	// comparo la palabra con cada uno de los comandos permitidos, hasta salir que es el ultimo
	while (i <= salir && strcmp(allCommands[i], palabra))
	{
		i++;
	}

	if (i > salir)
	{
		// Loo ingresado no coincide con alguno de los comandos, devuelvo error
		return -1;
	}
	else
	{
		// devuelvo el id del comando encontrado
		return i;
	}

}
